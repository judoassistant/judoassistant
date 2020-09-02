REM Publish windows build
:: ssh-keygen -f "C:\Drone\drone-runner-exec\id_ci" -P ""

if "%DRONE_BRANCH%"=="master" scp -i "C:\Drone\drone-runner-exec\id_ci" -o StrictHostKeyChecking=no "build-release\pkg\judoassistant-*-windows.exe" ci@judoassistant.com:"/var/www/builds.judoassistant.com/master"

if defined DRONE_TAG scp -i "C:\Drone\drone-runner-exec\id_ci" -o StrictHostKeyChecking=no "build-release\pkg\judoassistant-*-windows.exe" ci@judoassistant.com:"/var/www/builds.judoassistant.com/releases"

