# $Id: keywords.pl,v 1.1.1.1 2003/08/18 05:41:36 kaohj Exp $

# Generate keyword tables from ledman.h.

my @cmds;
my @leds;

while (<>) {
    if (/^#define\s+LEDMAN_CMD_([A-Z0-9_]+)/) {
	push @cmds, $1;
    } elsif (@cmds || /LEDMAN_MAX/) {
	next;
    } elsif (/^#define\s+LEDMAN_([A-Z0-9_]+)/) {
	push @leds, $1;
    }
}

print <<EOF;
typedef struct table {
  const char *name;
  unsigned value;
} table_t;

static const table_t cmds[] = {
EOF

print "  { \"$_\", LEDMAN_CMD_$_ },\n"
    foreach (@cmds);

print <<EOF;
  { 0, 0 }
};

static table_t leds[] = {
EOF

print "  { \"$_\", LEDMAN_$_ },\n"
    foreach (@leds);

print <<EOF;
  { 0, 0 }
};
EOF
