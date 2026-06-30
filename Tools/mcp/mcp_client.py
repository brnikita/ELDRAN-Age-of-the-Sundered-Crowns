"""Minimal MCP (HTTP streamable) client for the Unreal Editor MCP server (127.0.0.1:8000/mcp).
Handles the session handshake + SSE framing so we can drive the editor from the shell.

Usage:
  python mcp_client.py list_toolsets
  python mcp_client.py describe '{"toolset":"<name>"}'
  python mcp_client.py call '{"name":"<tool>","arguments":{...}}'   # via the call_tool meta-tool
  python mcp_client.py raw '{"name":"<tool>","arguments":{...}}'    # direct tools/call
"""
import json
import re
import sys
import urllib.request

URL = "http://127.0.0.1:8000/mcp"
HEADERS = {"Content-Type": "application/json", "Accept": "application/json, text/event-stream"}


def _post(body, sid=None):
    h = dict(HEADERS)
    if sid:
        h["Mcp-Session-Id"] = sid
    req = urllib.request.Request(URL, data=json.dumps(body).encode(), headers=h, method="POST")
    resp = urllib.request.urlopen(req, timeout=60)
    sid_out = resp.headers.get("Mcp-Session-Id", sid)
    raw = resp.read().decode("utf-8", "replace")
    # SSE framing: lines like "data: {...}". Grab the last JSON object.
    objs = re.findall(r"\{.*\}", raw, re.S)
    data = json.loads(objs[-1]) if objs else {}
    return data, sid_out


def session():
    init = {"jsonrpc": "2.0", "id": 1, "method": "initialize",
            "params": {"protocolVersion": "2024-11-05", "capabilities": {},
                       "clientInfo": {"name": "keldran", "version": "1"}}}
    _data, sid = _post(init)
    _post({"jsonrpc": "2.0", "method": "notifications/initialized"}, sid)
    return sid


def call(method, params, sid=None):
    if sid is None:
        sid = session()
    data, _ = _post({"jsonrpc": "2.0", "id": 2, "method": method, "params": params}, sid)
    return data


def tool(name, arguments=None, sid=None):
    return call("tools/call", {"name": name, "arguments": arguments or {}}, sid)


def _text(result):
    # MCP tool results are usually {content:[{type:text,text:...}]}
    try:
        parts = result["result"]["content"]
        return "\n".join(p.get("text", "") for p in parts if p.get("type") == "text")
    except Exception:
        return json.dumps(result, indent=2)


if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else "list_toolsets"
    arg = json.loads(sys.argv[2]) if len(sys.argv) > 2 else {}
    sid = session()
    if cmd == "list_toolsets":
        print(_text(tool("list_toolsets", {}, sid)))
    elif cmd == "describe":
        print(_text(tool("describe_toolset", arg, sid)))
    elif cmd == "call":
        print(_text(tool("call_tool", arg, sid)))
    elif cmd == "raw":
        print(_text(tool(arg["name"], arg.get("arguments", {}), sid)))
    else:
        print(_text(tool(cmd, arg, sid)))
