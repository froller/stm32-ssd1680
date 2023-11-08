#!/usr/bin/perl

use strict;
use warnings;

use Image::Magick;
use Data::Dumper;

my $gif = new Image::Magick;
my $e;
$e = $gif->Read($ARGV[0]) and die $e;

my $name = $ARGV[0];
$name =~ s/^.+\///g;
$name =~ s/\..+?$//g;

my ($m, $w, $h, $c) = $gif->Get(qw(magick width height colors));

my @pixels = $gif->GetPixels(map => 'I', width => $w, height => $h, normalize => 0);
undef $gif;

my %layers = (black => [], red => []);

for (my $i = 0; $i < scalar(@pixels); ++$i) {
  my $index = int($i / 8);
  my $bit = $i % 8;
  my ($k, $r) = ($pixels[$i] == 0xFFFF ? 1 : 0, $pixels[$i] == 10439 ? 1 : 0);
  $layers{'black'}[$index] |= ($k << (7 - $bit));
  $layers{'red'}[$index] |= ($r << (7 - $bit));
}


my $FF = *stdout;
printf $FF "/*\n * %s:%ux%ux%u\n */\n\n", $m, $w, $h, $c;
print $FF "const unsigned char ${name}_k[] = {\n";
for (my $y = 0; $y < $h; ++$y) {
  print $FF "  ";
  for (my $x = 0; $x < int($w / 8); ++$x) {
    printf $FF "0x%02X,", $layers{'black'}[$y * int($w / 8) + $x];
  }
  print "\n";
}
print $FF "};\n\n";

print $FF "const unsigned char ${name}_r[] = {\n";for (my $y = 0; $y < $h; ++$y) {
  print $FF "  ";
  for (my $x = 0; $x < int($w / 8); ++$x) {
    printf $FF "0x%02X,", $layers{'red'}[$y * int($w / 8) + $x];
  }
  print "\n";
}
print $FF "};\n\n";

