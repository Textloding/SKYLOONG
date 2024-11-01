#!/bin/bash

gzip -9kf ./web/favicon.ico
xxd -i ./web/favicon.ico.gz ./main/include/webserver/favicon.h

gzip -9kf ./web/index.html
xxd -i ./web/index.html.gz ./main/include/webserver/index.h

gzip -9kf ./web/index.js
xxd -i ./web/index.js.gz ./main/include/webserver/js.h

gzip -9kf ./web/ffmpeg.js
xxd -i ./web/ffmpeg.js.gz ./main/include/webserver/ffmpeg.h

gzip -9kf ./web/index.css
xxd -i ./web/index.css.gz ./main/include/webserver/css.h

gzip -9kf ./web/menu.jpg
xxd -i ./web/menu.jpg.gz ./main/include/webserver/menu.h

gzip -9kf ./web/close_eye.svg
xxd -i ./web/close_eye.svg.gz ./main/include/webserver/close_eye.h

gzip -9kf ./web/i.svg
xxd -i ./web/i.svg.gz ./main/include/webserver/i.h

gzip -9kf ./web/local.svg
xxd -i ./web/local.svg.gz ./main/include/webserver/local.h

gzip -9kf ./web/open_eye.svg
xxd -i ./web/open_eye.svg.gz ./main/include/webserver/open_eye.h

gzip -9kf ./web/wifi.svg
xxd -i ./web/wifi.svg.gz ./main/include/webserver/wifi.h

gzip -9kf ./web/arrow_left.png
xxd -i ./web/arrow_left.png.gz ./main/include/webserver/arrow_left.h

gzip -9kf ./web/arrow_right.png
xxd -i ./web/arrow_right.png.gz ./main/include/webserver/arrow_right.h

gzip -9kf ./web/cpu.png
xxd -i ./web/cpu.png.gz ./main/include/webserver/cpu.h

gzip -9kf ./web/demo.png
xxd -i ./web/demo.png.gz ./main/include/webserver/demo.h

gzip -9kf ./web/error_bg.png
xxd -i ./web/error_bg.png.gz ./main/include/webserver/error_bg.h

gzip -9kf ./web/error_m.png
xxd -i ./web/error_m.png.gz ./main/include/webserver/error_m.h

gzip -9kf ./web/ic_d.png
xxd -i ./web/ic_d.png.gz ./main/include/webserver/ic_d.h

gzip -9kf ./web/ic_del.png
xxd -i ./web/ic_del.png.gz ./main/include/webserver/ic_del.h

gzip -9kf ./web/image2.png
xxd -i ./web/image2.png.gz ./main/include/webserver/image2.h

gzip -9kf ./web/nothing.png
xxd -i ./web/nothing.png.gz ./main/include/webserver/nothing.h

gzip -9kf ./web/setting2.png
xxd -i ./web/setting2.png.gz ./main/include/webserver/setting2.h

gzip -9kf ./web/spead.png
xxd -i ./web/spead.png.gz ./main/include/webserver/spead.h

gzip -9kf ./web/theme1.png
xxd -i ./web/theme1.png.gz ./main/include/webserver/theme1.h

gzip -9kf ./web/theme2.png
xxd -i ./web/theme2.png.gz ./main/include/webserver/theme2.h

gzip -9kf ./web/theme3.png
xxd -i ./web/theme3.png.gz ./main/include/webserver/theme3.h

gzip -9kf ./web/time_bg.png
xxd -i ./web/time_bg.png.gz ./main/include/webserver/time_bg.h

gzip -9kf ./web/time_ic__.png
xxd -i ./web/time_ic__.png.gz ./main/include/webserver/time_ic.h

gzip -9kf ./web/video2.png
xxd -i ./web/video2.png.gz ./main/include/webserver/video2.h

gzip -9kf ./web/weather.png
xxd -i ./web/weather.png.gz ./main/include/webserver/weather.h

gzip -9kf ./web/assets/worker-43d19264.js
xxd -i ./web/assets/worker-43d19264.js.gz ./main/include/webserver/worker.h

rm ./web/*.gz
rm ./web/assets/*.gz

sed -i '1i\const '  ./main/include/webserver/favicon.h
sed -i '1i\const ' ./main/include/webserver/index.h
sed -i '1i\const ' ./main/include/webserver/js.h
sed -i '1i\const ' ./main/include/webserver/ffmpeg.h
sed -i '1i\const ' ./main/include/webserver/css.h
sed -i '1i\const ' ./main/include/webserver/menu.h
sed -i '1i\const ' ./main/include/webserver/close_eye.h
sed -i '1i\const ' ./main/include/webserver/i.h
sed -i '1i\const ' ./main/include/webserver/local.h
sed -i '1i\const ' ./main/include/webserver/open_eye.h
sed -i '1i\const ' ./main/include/webserver/wifi.h
sed -i '1i\const ' ./main/include/webserver/arrow_left.h
sed -i '1i\const ' ./main/include/webserver/arrow_right.h
sed -i '1i\const ' ./main/include/webserver/cpu.h
sed -i '1i\const ' ./main/include/webserver/demo.h
sed -i '1i\const ' ./main/include/webserver/error_bg.h
sed -i '1i\const ' ./main/include/webserver/error_m.h
sed -i '1i\const ' ./main/include/webserver/ic_d.h
sed -i '1i\const ' ./main/include/webserver/ic_del.h
sed -i '1i\const ' ./main/include/webserver/image2.h
sed -i '1i\const ' ./main/include/webserver/nothing.h
sed -i '1i\const ' ./main/include/webserver/setting2.h
sed -i '1i\const ' ./main/include/webserver/spead.h
sed -i '1i\const ' ./main/include/webserver/theme1.h
sed -i '1i\const ' ./main/include/webserver/theme2.h
sed -i '1i\const ' ./main/include/webserver/theme3.h
sed -i '1i\const ' ./main/include/webserver/time_bg.h
sed -i '1i\const ' ./main/include/webserver/time_ic.h
sed -i '1i\const ' ./main/include/webserver/video2.h
sed -i '1i\const ' ./main/include/webserver/weather.h
sed -i '1i\const ' ./main/include/webserver/worker.h
