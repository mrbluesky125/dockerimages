# Docker Services

Dieses Repository enthält Docker-Services, welche permanent auf den Servern laufen sollen.
Jeder Service ist in einem Unterordner durch min. eine Konfigurationsfile ("docker-compose.yml") definiert.

Zusätzlich können Images mit den dazugehörigen Dockerfiles ("Dockerfile") abgelegt werden.

## Volumes
Mögliche Volumes der Services werden in einen gleichnamigen Ordner unter "/srv/docker/volumes/" gemapped.