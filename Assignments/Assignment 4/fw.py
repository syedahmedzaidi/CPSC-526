# CPSC 526 - Assignment 4
# Authors: Andrejs Gusakovs, Syed Ahmed Zaidi

import ipaddress
import sys
import string
import re

# Main function - reads given argument (filename) and calls the according methods
def main():
    fileName = sys.argv[1]

    rules = readRules(fileName)

    packets = readPackets(rules)

# Reads the packets from the stdin and calls method to decide which action to apply to the packets according to the rules
def readPackets(rules):
    for line in sys.stdin:
        accept = False
        packet = line.split()
        for rule in rules:
            accept = decideAction(rule, packet)
            if(accept):
                print(rule[1]+"("+str(rules.index(rule)+1)+") "+packet[0]+' '+packet[1]+' '+packet[2]+' '+packet[3])
                break
        else:
            print("drop() "+packet[0]+' '+packet[1]+' '+packet[2]+' '+packet[3])

# Method to decide if action is valid (i.e. there is a rule for it)
def decideAction(rule, packet):
    if(len(rule) <= 1):
      return False
    address = packet[1]
    try:
        address = ipaddress.ip_address(address)        
    except ValueError as e:
        return False
    valid = False
    valid = (packet[0]==rule[0])
    valid = ((rule[2]=='*') or (ipaddress.ip_address(packet[1]) in ipaddress.ip_network(rule[2], False))) and valid
    valid = ((packet[2] in rule[3]) or (rule[3][0] == '*')) and valid
    if(len(rule) == 5):
        valid = (packet[3] == str(1)) and valid
    return valid

# Read the document for the rules and check for comments within the file
def readRules(fileName):
    with open(fileName) as f:
        contents = [c.strip() for c in f.readlines()]
        index = 0
        for c in contents:
            if(re.search(r'[#]',c) is not None):
                 contents[index] = c.replace(' ',"")
            index = index + 1
        rules = [c.split() for c in contents]
    for r in rules:
        if(len(r) > 3):
            r[3] = r[3].split(',')
    return rules

if __name__ == '__main__':
    main()
