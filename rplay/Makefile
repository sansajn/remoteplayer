IMAGE_NAME = rplay-dev
DOCKER_RUN = docker run --rm \
	--user $(shell id -u):$(shell id -g) \
	-v $(PWD):/src -w /src \
	-e HOME=/tmp \
	$(IMAGE_NAME)

# creates development docker image
image:
	docker build -t $(IMAGE_NAME) -f Dockerfile.dev .

# creates runtime docker image
runtime:
	docker build -t rplay-runtime -f Dockerfile.runtime .

build:
	$(DOCKER_RUN) scons -j8

clean:
	$(DOCKER_RUN) scons -c

bash:
	$(DOCKER_RUN) bash
