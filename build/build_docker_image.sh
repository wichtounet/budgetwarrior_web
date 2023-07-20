#!/bin/bash

make clean
docker run -v $(pwd):/home/budgetwarrior_web -w /home/budgetwarrior_web -t budgetwarrior:build make -j9 release_debug
docker build -f build/Dockerfile -t budgetwarrior:web .
docker tag budgetwarrior:web wichtounet/budgetwarrior:web
docker push wichtounet/budgetwarrior:web
