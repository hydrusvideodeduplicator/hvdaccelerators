FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python-is-python3 \
    python3-venv \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir /hvdaccelerators
WORKDIR /hvdaccelerators

RUN python -m venv /opt/venv
# Enable venv
ENV PATH="/opt/venv/bin:$PATH"
