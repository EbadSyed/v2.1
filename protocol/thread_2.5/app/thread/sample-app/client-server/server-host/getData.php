<?php

// Grab the last 180 lines from the reports.csv file then convert it
// to an array and json encode it for passing back to browser.
//
// Each line within the reports.csv file should be formatted as follows:
//   <local time>, <remote IPv6 address>, <data>
//
// For example
//   123,fd01::ee:22:1,4567
//   235,fd01::238:3,9858

$csv = `tail -180 reports.csv`;
$array = array_map("str_getcsv", explode("\n", $csv));
echo json_encode($array);

?>
