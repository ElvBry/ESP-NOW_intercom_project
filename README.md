

Before you start working each day (or before you start a new feature), make sure you have the freshest main:
1. git checkout main        # switch to the main branch
2. git pull origin main     # fetch & merge what’s on GitHub

Rather than working directly on main, you make a new branch for your task. This keeps your work isolated until you’re ready to merge.


Naming convention: feature/… for new stuff, bugfix/… for fixes, docs/… for documentation changes.
1. git checkout -b feature/blink-led # -b both creates and switches to the new branch.

make changes, commit (message with ) and push:
1. git add components/my_component/include/my_component.h main/blink.c
alternativly
1. git add -A #all changes in repo
2. git commit -m "feat(blink): add adjustable blink period via Kconfig"
3. git push -u origin feature/blink-led

after feature is done click merge on github
delete branch locally & remotely:
1. git checkout main
2. git branch -d feature/blink-led
3. git push origin --delete feature/blink-led

go back to main and pull in the merge
1. git checkout main
2. git pull

DONE


If first configuration or after change in sdkconfig.defaults

1. git checkout main
2. git pull origin main

after pulling from git you should
1. ctrl + shift + p and select ESP-IDF: Open ESP-IDF Terminal
2. enter "del sdkconfig"
3. enter "idf.py" reconfigure
4. afterwards enter "idf.py build flash" as normal 

idf.py commands do not work in powershell, use ESP-IDF Terminal