# library directory mounted as /music for `run` target
LIBRARY_PATH = $(HOME)/Music

IMAGE_NAME = rplay-dev
RUNTIME_IMAGE = rplay-runtime

DOCKER_RUN_BUILD = docker run --rm \
	--user $(shell id -u):$(shell id -g) \
	-v $(PWD):/src -w /src \
	-e HOME=/tmp \
	$(IMAGE_NAME)

AUDIO_GID := $(shell getent group audio | cut -d: -f3)

DOCKER_RUN_RUN = docker run --rm -it --init \
	--user $(shell id -u):$(shell id -g) \
	--device /dev/snd \
	--group-add $(AUDIO_GID) \
	-v "$(PWD)":/build -w /build \
	-v "$(PWD)/asound.conf":/etc/asound.conf:ro \
	-v "$(LIBRARY_PATH)":/music:ro \
	--publish 23333-23335:23333-23335 \
	-e HOME=/tmp

# creates development docker image
image:
	docker build -t $(IMAGE_NAME) -f Dockerfile.dev .

build:
	$(DOCKER_RUN_BUILD) scons -j8

clean:
	$(DOCKER_RUN_BUILD) scons -c

run:
	@if [ -z "$(AUDIO_GID)" ]; then echo "Could not resolve 'audio' group GID on host."; exit 1; fi
	$(DOCKER_RUN_RUN) $(IMAGE_NAME) ./rplay --library=/music

# Join a running runtime container
join:
	@cid=$$(docker ps --filter "ancestor=$(IMAGE_NAME)" --format "{{.ID}}"); \
	if [ -z "$$cid" ]; then \
		echo "No running container found for image $(IMAGE_NAME)"; \
		exit 1; \
	fi; \
	echo "Opening shell in container $$cid..."; \
	docker exec --interactive --tty $$cid bash

# Stop a running development container
stop:
	@cid=$$(docker ps --filter "ancestor=$(IMAGE_NAME)" --format "{{.ID}}"); \
	if [ -z "$$cid" ]; then \
		echo "No running container found for image $(IMAGE_NAME)"; \
		exit 1; \
	fi; \
	echo "Stopping container $$cid..."; \
	docker stop $$cid

# interactive bash for debugging
bash:
	$(DOCKER_RUN_RUN) $(IMAGE_NAME) bash


# creates runtime docker image
runtime-image:
	docker build -t $(RUNTIME_IMAGE) -f Dockerfile.runtime .

runtime-run:
	@if [ -z "$(AUDIO_GID)" ]; then echo "Could not resolve 'audio' group GID on host."; exit 1; fi
	$(DOCKER_RUN_RUN) $(RUNTIME_IMAGE) ./rplay --library=/music

# interactive bash for debugging
runtime-bash:
	@if [ -z "$(AUDIO_GID)" ]; then echo "Could not resolve 'audio' group GID on host."; exit 1; fi
	$(DOCKER_RUN_RUN) $(RUNTIME_IMAGE) bash

