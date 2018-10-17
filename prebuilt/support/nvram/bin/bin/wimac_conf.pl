#!/usr/bin/perl

sub Output
{
	my ($var, $value) = @_;
	print FD "#ifndef $var\n";
	print FD "#define $var $value\n";
	print FD "#else\n";
	print FD "#undef $var\n";
	print FD "#define $var $value\n";
	print FD "#endif\n\n";
}

($name, $config) = @ARGV;
$config =~ s/\'/"/g;
@config = split / /, $config;

open FD, ">include/conf_$name.h";
print FD "/** \n";
print FD " *	\@file conf_$name.h \n";
print FD " */ \n";
print FD "#ifndef conf_$name" . "_h\n";
print FD "#define conf_$name" . "_h\n\n";

for ($i = 0; $i < @config; $i++) {
	$_ = $config[$i];
	s/-D//g;
	if ($_){
		if (/=/){
			($var, $value) = ($`, $');
			if(($value =~ y/"//) == 1) {
				do {
					$value .= " " . $config[++$i];
				} while ($i < @config && !($config[$i] =~ /"/));
			}
			Output($var, $value);
		} else {
			Output($_, 1);
		}
	}
}

print FD "#endif\n";
close FD;

