#!/bin/bash

# SETUP


INCLUDES=../.INCLUDES
MAINS=($(ls mains | cut -f1 -d'_'))
RUN="yes"
OK="\xE2\x9C\x85"
DEAD="\xE2\x98\xA0"
NOPE="\xE2\x9D\x8C"
WARNING="\xE2\x9A\xA0"
CONT="$(pwd | rev | cut -f1 -d'/' | rev)"

RESET="\033[0m"
BOLD="\033[1m"
BLINK="\033[5m"

RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
PURPLE="\033[35m"

# FUNCTIONS

run_tests ()
{
	rm logs/"$1"_ft.error.log logs/"$1"_std.error.log
	g++ -Werror -Wextra -Werror -std=c++98 -I . -I "$INCLUDES" -I "$INCLUDES"/EMPTY -I .. -I ../../../includes -I "$INCLUDES"/EMPTY/JUST_IN_CASE \
			mains/"$1"_main.cpp -o bin/"$1"_ft 2> logs/"$1"_ft.error.log
	g++ -Werror -Wextra -Werror -std=c++98 -D STD -I . -I "$INCLUDES" -I "$INCLUDES"/EMPTY \
			-I "$INCLUDES"/EMPTY/JUST_IN_CASE -I .. -I ../../../includes mains/"$1"_main.cpp -o bin/"$1"_std 2> logs/"$1"_std.error.log

	if [ $(cat logs/"$1"_std.error.log | wc -c) != "0" ]
	then
		echo -e "$RED$BLINK$WARNING\t$RESET$PURPLE$BOLD$1_main.cpp seems to be invalid!$RESET"
	else
		if [ "$(cat logs/"$1"_ft.error.log | wc -c | rev | cut -f1 -d' ')" != "0" ]
		then
			echo -e "$BOLD$RED\033[25m$NOPE\t$1 test does not compile!$RESET"
		else
			rm logs/"$1"_ft.error.log
			./bin/"$1"_std > out/"$1"_std.log 2> stderror/"$1"_std
			timeout 5 ./bin/"$1"_ft > out/"$1"_ft.log 2> stderror/"$1"_ft
			RET=$?
			diff out/"$1"_std.log out/"$1"_ft.log > diff/"$1".diff
			#echo "ret:$RET"
			if [ $RET -eq 124 ]
			then
				echo -e "$RED$NOPE\t$1 timeout. No out provided.$RESET"
			elif [ $RET -eq 134 ]
			then
				echo -e "$RED$NOPE\t$1 free() : invalid pointer. No out provided.$RESET"
			elif [ $RET -eq 139 ]
			then
				echo -e "$RED$NOPE\t$1 Segfault detected. No out provided.$RESET"
			elif [ $(cat diff/"$1".diff | wc -l) -eq 0 ]
			then
				echo -e "$OK$BOLD$GREEN\t$1 test passed!$RESET"
				rm diff/"$1".diff out/"$1"_std.log out/"$1"_ft.log stderror/"$1"_std stderror/"$1"_ft
			else
				echo -e "$RED$NOPE\t$1 test outputs differ. Check logs$RESET"
				if [ $(cat stderror/"$1"_std | wc -c) == "0" ]
				then
					rm stderror/"$1"_std
				fi
				if [ $(cat stderror/"$1"_ft | wc -c) == "0" ]
				then
					rm stderror/"$1"_ft
				fi
			fi
		fi
		rm logs/"$1"_std.error.log
	fi
	rm bin/"$1"_ft bin/"$1"_std
}

# CHECK OPTIONS


for VAR in "$@"
do
	if [ "$VAR" = "clean" ]
	then
		RUN="no"
	fi
	#other arguments
done


# RUN

rm -rf out bin diff leak logs stderror
if [ $RUN = "no" ]
then
	exit
fi

mkdir out
mkdir diff
mkdir bin
mkdir leak
mkdir logs
mkdir stderror

echo -e "$BLUE$BOLD-" $CONT ":\n$RESET"
echo -en "$YELLOW Found ${#MAINS[@]} tests for $CONT, "
if (( ${#MAINS[@]} > 0 ))
then
	echo -e running tests :"\n$RESET"
else
	echo -e jumping to next container."\n$RESET"
fi

for M in "${MAINS[@]}"
do
	run_tests $M $1 2> /dev/null
done
echo
rm -r bin
