#!/bin/bash

# arguments need to be passed in  key value pairs separated by spaces
# example ./Jsonify.sh hello hola world mundo "mi casa" "tu casa"

ARGS=("$@")
NARGS=${#ARGS[@]}

# create json with arguments
SEP=":"
JSON="{\n"
for i in "${!ARGS[@]}"; do
	arg="${ARGS[$i]}"
	if [[ $SEP = ":" ]]; then
    	JSON="$JSON\t\"${arg}\": "
    fi
    if [[ $SEP = "," ]]; then
    	if [[ $i -eq ${NARGS}-1 ]]; then
    		JSON="$JSON\"${arg}\"\n"
    	else
    		JSON="$JSON\"${arg}\",\n"
    	fi
    fi
    if [[ $SEP = ":" ]]; then
    	SEP=","
    else
    	SEP=":"
    fi
done
JSON="$JSON}"
# print to stdout
echo -e $JSON