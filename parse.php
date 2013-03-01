<?php

error_reporting(E_ALL ^ E_NOTICE);

$lines = file($argv[1]);

$gets = array();
$puts = array();
$map = array();

function hex_to_fname($hex) {
	global $map;

	if (empty($hex))
		return;

	if (in_array($hex, $map)) {
		return $map[$hex];
	} else {
		$exec = exec('../../../../utils/backtrace ../../kernel.o '.$hex);
		$fname = explode(' ', $exec);
		$map[$hex] = $fname[1];
		return $map[$hex];
	}
}

$boring = array('malloc', 'pagedir_create', 'calloc', 'pde_create',
	            'bitmap_create', 'free', 'pagedir_destroy', 'realloc',
	            'pagedir_set_page', 'load_segment', 'lookup_page',
                    'palloc_get_page', 'palloc_free_page');

function get_interesting($stack_arr) {
	global $boring;

	foreach ($stack_arr as $i => $stack_item) {
		$name = hex_to_fname($stack_item);

		if (!in_array($name, $boring)) {
			$caller = hex_to_fname($stack_arr[$i + 1]);
			return array($name, $caller);
		}
	}
}

foreach ($lines as $i => $line) {
	if ($i % 1000 == 0) echo "Alive!\n";

	$pieces = explode(': ', $line);
	if (count($pieces) != 2 || !($pieces[0] == 'GET' || $pieces[0] == 'FREE'))
		continue;

	$trace = explode (' ', $pieces[1]);
	$interesting = get_interesting($trace);

	if (!is_array($gets[$interesting[0]]))
		$gets[$interesting[0]] = array();

	if ($pieces[0] == 'GET') {
		$gets[$interesting[0]][0]++;
		$gets[$interesting[0]][1] = $interesting[1];
	} else {
		$puts[$interesting[0]][0]++;
		$puts[$interesting[0]][1] = $interesting[1];
	}
}

echo "Gets:\n";
foreach ($gets as $k => $v) {
	if (!empty($v[0])) {
		$totgets += intval($v[0]);
		echo "{$k}: {$v[0]} (called by {$v[1]})\n";
	}
}
echo "TOTAL GETS: {$totgets}\n";

echo "\n\n\nFrees:\n";
foreach ($puts as $k => $v) {
	if (!empty($v[0])) {
		$totfrees += intval($v[0]);
		echo "{$k}: {$v[0]} (called by {$v[1]})\n";
	}
}
echo "TOTAL FREES: {$totfrees}\nDifference: ", ($totgets - $totfrees), "\n";
