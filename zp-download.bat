del "zapret-*"
mkdir "zapret-latest"
curl -L "https://github.com/Flowseal/zapret-discord-youtube/releases/download/1.9.0b/zapret-discord-youtube-1.9.0b.zip" -O "zapret-discord-youtube-1.9.0b.zip"
move "D:\zapret-discord-youtube-1.9.0b.zip" "D:\zapret-latest\zapret-discord-youtube-1.9.0b.zip"
cd zapret-latest
tar -xvf zapret-discord-youtube-1.9.0b.zip
del "zapret-*.zip"