#!/bin/bash

gzip -9kf ./web/css/csss.css
xxd -i ./web/css/csss.css.gz ./main/include/webserver/csss.h

gzip -9kf ./web/js/jq.js
xxd -i ./web/js/jq.js.gz ./main/include/webserver/jq.h

gzip -9kf ./web/js/mdb.js
xxd -i ./web/js/mdb.js.gz ./main/include/webserver/mdb.h

gzip -9kf ./web/index.htm
xxd -i ./web/index.htm.gz ./main/include/webserver/index.h

gzip -9kf ./web/favicon.ico
xxd -i ./web/favicon.ico.gz ./main/include/webserver/favicon.h
rm ./web/*.gz
rm ./web/js/*.gz
rm ./web/css/*.gz
sed -i '1i\const '  ./main/include/webserver/csss.h
sed -i '1i\const ' ./main/include/webserver/jq.h
sed -i '1i\const ' ./main/include/webserver/mdb.h
sed -i '1i\const ' ./main/include/webserver/index.h
sed -i '1i\const ' ./main/include/webserver/favicon.h
