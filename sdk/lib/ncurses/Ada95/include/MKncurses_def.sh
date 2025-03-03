#! /bin/sh
# $Id: MKncurses_def.sh,v 1.1 2011/08/18 02:20:38 tsaitc Exp $
##############################################################################
# Copyright (c) 2000 Free Software Foundation, Inc.                          #
#                                                                            #
# Permission is hereby granted, free of charge, to any person obtaining a    #
# copy of this software and associated documentation files (the "Software"), #
# to deal in the Software without restriction, including without limitation  #
# the rights to use, copy, modify, merge, publish, distribute, distribute    #
# with modifications, sublicense, and/or sell copies of the Software, and to #
# permit persons to whom the Software is furnished to do so, subject to the  #
# following conditions:                                                      #
#                                                                            #
# The above copyright notice and this permission notice shall be included in #
# all copies or substantial portions of the Software.                        #
#                                                                            #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    #
# THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    #
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        #
# DEALINGS IN THE SOFTWARE.                                                  #
#                                                                            #
# Except as contained in this notice, the name(s) of the above copyright     #
# holders shall not be used in advertising or otherwise to promote the sale, #
# use or other dealings in this Software without prior written               #
# authorization.                                                             #
##############################################################################
#
# MKncurses_def.sh -- generate fallback definitions for ncurses_cfg.h
#
# Author: Thomas E. Dickey 2000
#
# Given the choice between constructs such as
#
#	#if defined(foo) && foo
#	#if foo
#
# we chose the latter.  It is guaranteed by the language standard, and there
# appear to be no broken compilers that do not honor that detail.  But some
# people want to use gcc's -Wundef option (corresponding to one of the less
# useful features in Watcom's compiler) to check for misspellings.  So we
# generate a set of fallback definitions to quiet the warnings without making
# the code ugly.
#
DEFS="${1-ncurses_defs}"
cat <<EOF
/*
 * This file is generated by $0
 */

#ifndef NC_DEFINE_H
#define NC_DEFINE_H 1

EOF

${AWK-awk} <$DEFS '
!/^[@#]/ {
	if ( NF == 1 )
	{
		print "#ifndef", $1
		print "#define", $1, "0"
		print "#endif"
		print ""
	} else if ( NF != 0 ) {
		print "#ifndef", $1
		printf "#define"
		for (n = 1; n <= NF; n++) {
			printf " %s", $n
		}
		print ""
		print "#endif"
		print ""
	}
}
END	{
print "#endif /* NC_DEFINE_H */"
	}
'
