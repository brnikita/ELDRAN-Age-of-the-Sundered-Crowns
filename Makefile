# KELDRAN — top-level make targets.
# Content generation pipeline (Docs/03_AIContentPipeline.md). Keys come from the root .env.

PY ?= python
GENDIR := Tools/gen

.PHONY: generate generate-dry gen-text gen-images gen-audio gen-deps backend-up backend-down

## Install generator Python deps
gen-deps:
	$(PY) -m pip install -r $(GENDIR)/requirements.txt

## Full content generation (text -> images -> audio). Idempotent; skips cached items.
generate: gen-text gen-images gen-audio
	@echo "[make] generation complete."

## Cost/plan preview — makes NO API calls.
generate-dry:
	DRYRUN=1 $(PY) $(GENDIR)/gen_text.py
	DRYRUN=1 $(PY) $(GENDIR)/gen_images.py
	DRYRUN=1 $(PY) $(GENDIR)/gen_audio.py

gen-text:
	cd $(GENDIR) && $(PY) gen_text.py
gen-images:
	cd $(GENDIR) && $(PY) gen_images.py
gen-audio:
	cd $(GENDIR) && $(PY) gen_audio.py

## Backend stack
backend-up:
	cd Backend && docker compose up -d --build
backend-down:
	cd Backend && docker compose down
