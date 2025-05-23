#!/bin/bash

docker run -v $(pwd):/home/budgetwarrior_web -w /home/budgetwarrior_web -t wichtounet/budgetwarrior:build make clean
docker run -v $(pwd):/home/budgetwarrior_web -w /home/budgetwarrior_web -t wichtounet/budgetwarrior:build make -j17 release_debug
docker build -f build/Dockerfile -t budgetwarrior:web .
docker tag budgetwarrior:web wichtounet/budgetwarrior:web
docker push wichtounet/budgetwarrior:web
