IMAGE_NAME = rplay-dev
DOCKER_RUN = docker run --rm \
	--user $(shell id -u):$(shell id -g) \
	-v $(PWD):/src -w /src \
	-e HOME=/tmp \
	$(IMAGE_NAME)

RUNTIME_IMAGE = rplay-runtime
AUDIO_GID := $(shell getent group audio | cut -d: -f3)

# creates development docker image
image:
	docker build -t $(IMAGE_NAME) -f Dockerfile.dev .

# creates runtime docker image
runtime-image:
	docker build -t $(RUNTIME_IMAGE) -f Dockerfile.runtime .

build:
	$(DOCKER_RUN) scons -j8

clean:
	$(DOCKER_RUN) scons -c

# interactive bash for debugging
bash:
	$(DOCKER_RUN) bash

run:
	@if [ -z "$(AUDIO_GID)" ]; then echo "Could not resolve 'audio' group GID on host."; exit 1; fi
	docker run --rm -it \
		--user $(shell id -u):$(shell id -g) \
		--device /dev/snd \
		--group-add $(AUDIO_GID) \
		-v $(PWD):/build -w /build \
		-v "$(PWD)/asound.conf":/etc/asound.conf:ro \
		-e HOME=/tmp \
		$(RUNTIME_IMAGE) ./rplay

# interactive bash for debugging
run-bash:
	@if [ -z "$(AUDIO_GID)" ]; then echo "Could not resolve 'audio' group GID on host."; exit 1; fi
	docker run --rm -it \
		--user $(shell id -u):$(shell id -g) \
		--device /dev/snd \
		--group-add $(AUDIO_GID) \
		-v $(PWD):/build -w /build \
		-v "$(PWD)/asound.conf":/etc/asound.conf:ro \
		-e HOME=/tmp \
		$(RUNTIME_IMAGE) bash
