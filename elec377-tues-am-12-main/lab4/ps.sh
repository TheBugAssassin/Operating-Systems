#!/bin/bash

# ELEC377 - Operating System
# Lab 4 - Shell Scripting, ps.sh
# Program Description:

# Initialize variables to default 'no' for flags
showRSS="no"
showGROUP="no"
showCOMM="no"
showCOMMAND="no"

# Loop through command line arguments to set flags
while (($# > 0)) ; do
    # Check each argument to set the corresponding flag
    if [[ $1 == "-rss" ]]; then
        showRSS="yes"
    elif [[ $1 == "-group" ]]; then
        showGROUP="yes"
    elif [[ $1 == "-comm" ]]; then
        showCOMM="yes"
    elif [[ $1 == "-command" ]]; then
        showCOMMAND="yes"
    else
        # Exit with an error message for unrecognized flag
        echo "Error: Unrecognized flag '$1'"
        exit 1
    fi
    # Check for mutually exclusive flags (-comm and -command)
    if [[ $1 == "-comm" || $1 == "-command" ]]; then
        if [[ $showCOMM == "yes" && $showCOMMAND == "yes" ]]; then
            echo "Error: Both -comm and -command flags cannot be used together."
            exit 1
        fi
    fi
    shift # Move to the next Arguement
done

# Output variable values to confirm the options set
#echo "showRSS = $showRSS"
#echo "showGROUP = $showGROUP"
#echo "showCOMM = $showCOMM"

# Header printing
printf "%-25s" "PID"
printf "%-25s" "UID"

    # Printing columns conditionally based on flags
    if [[ $showGROUP == "yes" ]] ; then
        printf "%-25s" "GID"
    fi
    if [[ $showRSS == "yes" ]] ; then
       printf "%-25s" "RSS"
    fi
    if [[ $showCOMMAND == "yes" || $showCOMM == "yes" ]] ; then
        printf "%-25s" "Command"
    fi

printf "\n"
echo "-------------------------------------------------------------------------------------------"

# Iterate through /proc directories representing processes and write information to temporary file
tmpFile="/tmp/tmpPs$$.txt"
for p in /proc/[0-9]*/; do
    if [[ -d "$p" ]]; then
        #echo "Process directory is $p"

        # Extracting PID from the status files
        pid=$(grep '^Pid' "$p/status" | sed 's/^Pid:[[:space:]]*\([0-9]*\).*/\1/')
        #echo "PID: $pid"
        
        # Extracting numeric User ID from status files
        uid=$(grep '^Uid' "$p/status" | sed 's/^Uid:[[:space:]]*\([0-9]*\).*/\1/')
        #echo "UID: $uid"
        
        # Extracting numeric Group ID from status files
        gid=$(grep '^Gid' "$p/status" | sed 's/^Gid:[[:space:]]*\([0-9]*\).*/\1/')
        #echo "GID: $gid"
        
        # Extracting RSS from status files
        rss=$(grep '^VmRSS' "$p/status" | sed 's/^VmRSS:[[:space:]]*\([0-9]*\).*/\1/')
        # Adjust RSS for kernel processes
        if [ -z "$rss" ]; then
            rss="0"
        fi
        #echo "RSS: $rss"
        
        # Extracting short command name
        command=$(grep '^Name' "$p/status" | sed 's/^Name:\s*//')
        #echo "Command Name: $command"

        # Extracting command line with arguments
        cmdline=$(tr '\0' ' ' < "$p/cmdline")
        if [ -z "$cmdline" ]; then
            cmdline=$command
        fi
        #echo "Command Line: $cmdline"

         #convert user ID and group ID to symbolic names such as root, and netid to username
        username=$(grep "^.*:x:$uid:" /etc/passwd | cut -d: -f1)
        groupName=$(grep ":x:$gid:" /etc/group | cut -d: -f1)

        # Writing process information to temporary file
        printf "%-25s" $pid>>$tmpFile
        printf "%-25s" $username>>$tmpFile

        # Append Group Name if -group flag is set
        if [[ $showGROUP == "yes" ]]; then
            printf "%-25s" $groupName>>$tmpFile
        fi

        # Append RSS if -rss flag is set
        if [[ $showRSS == "yes" ]]; then
            printf "%-25s" $rss>>$tmpFile
        fi

        # Append Command/Command Line if -command or -comm flags are set
        if [[ $showCOMM == "yes" || $showCOMMAND == "yes" ]]; then
            printf "%-25s" $command>>$tmpFile
        fi
        # Add newline to seperate
        printf "\n">>$tmpFile
    fi
done

# Sort the temporary file numerically based on the first column (PID) and display the result
sort -n -k1 $tmpFile
rm $tmpFile # Remove the temporary file after use