If first configuration or after change in sdkconfig.defaults

cmd: git pull origin main

after pulling from git you should
1. ctrl + shift + p and select ESP-IDF: Open ESP-IDF Terminal
2. enter "del sdkconfig"
3. enter "idf.py" reconfigure
4. afterwards enter "idf.py build flash" as normal 

idf.py commands did not work in powershell, use ESP-IDF Terminal