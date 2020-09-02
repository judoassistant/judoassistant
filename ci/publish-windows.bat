REM Publish windows build

ssh -i "C:\Users\Svend\.ssh\id_ci" "judoassistant-0.3.0-windows.exe" ci@judoassistant.com:/var/www/builds.judoassistant.com/master
