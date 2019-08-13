Simple Get Server
---
Simple Get Server aims to be a very simple and fast web server. Simple Get
Server only accepts GET requests and should be used for static file serving.
This server aims to be a MVP (minimal viable program). An MVP is a program that
has the minimal amount of code/features need to accomplish the desired task.
What is the minimal code can be a bit iffy as this program contains some
security checks that might be seen as unneccessary features, and thus make it
not an MVP. I feel that these security features are neccessary for an internet
facing program and thus have included them in the base program. Future patches
for this program will be released to add more features to the server while
leaving the choice of having the feature up to the system admin.

Simple Get Server is programmed for Unix systems and thus only works on
Unix/Linux systems.

## Config
To keep the code in Simple Get Server concise, the settings of the server are
written directly in the code. In the main folder, you will find a config.h file.
This file contains all the code that a user may wish to modify. Simply change
the defines in the file and recompile the server.

## Build
This project consists of two files, main.c and config.h. You can easily build
this project with console commands or by the provided cmake file.

