# Docker Services

Dieses Repository enthält Docker-Services, welche permanent auf den Servern laufen sollen.
Jeder Service ist in einem Unterordner durch mindestens eine 
Konfigurationsfile ("docker-compose.yml") in der Syntax-Version 2 definiert.

Zusätzlich können Images, definiert durch Dockerfiles ("Dockerfile"), 
plus die zugehörigen Skripte, abgelegt werden.

## Volumes
Mögliche Volumes der Services werden in einen gleichnamigen Ordner 
unter "/srv/docker/volumes/" gemapped.