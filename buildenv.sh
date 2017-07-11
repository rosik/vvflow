#!/dev/null

mkdir -p build
sudo docker build -t dovvbuild:latest $PWD && \
sudo docker run -it --rm \
    -v $PWD:/vvflow:ro \
    -v $PWD/build:/root \
    -p 1207:1207 \
    dovvbuild:latest /bin/bash

# Ronn cheatsheet:
# http://ricostacruz.com/cheatsheets/ronn.html

# python -m SimpleHTTPServer 1207 &
# grip /vvflow/README.md 0.0.0.0:1207 &
