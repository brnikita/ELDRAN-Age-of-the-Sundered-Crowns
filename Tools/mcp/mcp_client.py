"""MCP client for the Unreal Editor MCP server (127.0.0.1:8000/mcp).

The server speaks streamable-HTTP: initialize returns JSON; tool calls stream the result
back on the POST connection as SSE (`event: message` / `data: {...}`). We use curl -N for
robust streaming (urllib buffers poorly here).

Usage:
  python mcp_client.py list_toolsets
  python mcp_client.py describe_toolset '{"toolset_name":"UMGToolSet.UMGToolSet"}'
  python mcp_client.py call_tool '{"tool_name":"...","arguments_json":"{...}"}'
"""
import json
import subprocess
import sys

URL = "http://127.0.0.1:8000/mcp"
TIMEOUT = 120


def _curl(args, body, timeout=TIMEOUT):
    cmd = ["curl", "-s", "-N", "-m", str(timeout), "-X", "POST", URL,
           "-H", "Content-Type: application/json",
           "-H", "Accept: application/json, text/event-stream"] + args + \
          ["-d", json.dumps(body)]
    out = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout + 10)
    return out.stdout


def _curl_headers(body):
    cmd = ["curl", "-s", "-D", "-", "-o", "NUL", "-m", "15", "-X", "POST", URL,
           "-H", "Content-Type: application/json",
           "-H", "Accept: application/json, text/event-stream",
           "-d", json.dumps(body)]
    out = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    for line in out.stdout.splitlines():
        if line.lower().startswith("mcp-session-id:"):
            return line.split(":", 1)[1].strip()
    return None


def session():
    sid = _curl_headers({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                         "params": {"protocolVersion": "2024-11-05", "capabilities": {},
                                    "clientInfo": {"name": "keldran", "version": "1"}}})
    if not sid:
        sys.exit("no session id — is the editor (MCP server) running?")
    _curl(["-H", f"Mcp-Session-Id: {sid}"],
          {"jsonrpc": "2.0", "method": "notifications/initialized"}, timeout=10)
    return sid


def parse_sse(raw):
    """Return the JSON payload from either an SSE stream or a plain JSON response."""
    result = None
    for line in raw.splitlines():
        if line.startswith("data:"):
            payload = line[5:].strip()
            try:
                result = json.loads(payload)
            except json.JSONDecodeError:
                pass
    if result is None and raw.lstrip().startswith("{"):
        try:
            result = json.loads(raw)
        except json.JSONDecodeError:
            pass
    return result


def tool(name, arguments, sid=None, timeout=TIMEOUT):
    if sid is None:
        sid = session()
    raw = _curl(["-H", f"Mcp-Session-Id: {sid}"],
                {"jsonrpc": "2.0", "id": 7, "method": "tools/call",
                 "params": {"name": name, "arguments": arguments or {}}}, timeout)
    data = parse_sse(raw)
    if not data:
        return f"<no response; raw head: {raw[:200]!r}>"
    if "error" in data:
        return "ERROR: " + json.dumps(data["error"])
    try:
        parts = data["result"]["content"]
        text = "\n".join(p.get("text", "") for p in parts if p.get("type") == "text")
        if data["result"].get("isError"):
            text = "TOOL ERROR:\n" + text
        return text
    except Exception:
        return json.dumps(data, indent=2)


def call(toolset, tool_name, arguments=None, sid=None, timeout=TIMEOUT):
    """Invoke a toolset tool through the call_tool meta-tool."""
    args = {"tool_name": tool_name, "arguments": arguments or {}}
    if toolset:
        args["toolset_name"] = toolset
    return tool("call_tool", args, sid, timeout)


if __name__ == "__main__":
    # Forms:
    #   mcp_client.py list_toolsets
    #   mcp_client.py describe_toolset '{"toolset_name":"..."}'
    #   mcp_client.py <toolset_name> <tool_name> ['<json args>']
    if len(sys.argv) >= 3 and not sys.argv[2].lstrip().startswith("{"):
        ts, tn = sys.argv[1], sys.argv[2]
        args = json.loads(sys.argv[3]) if len(sys.argv) > 3 else {}
        print(call(ts, tn, args))
    else:
        name = sys.argv[1] if len(sys.argv) > 1 else "list_toolsets"
        args = json.loads(sys.argv[2]) if len(sys.argv) > 2 else {}
        print(tool(name, args))
