@echo off
wsl --cd /home/tengeroff/ESPHome-Wohnraumlueftung bash -c "if [ -f version_bump.py ]; then python3 version_bump.py; fi && ~/esphome-venv/bin/esphome compile %*"
