#!/usr/bin/php
<?php

/**
 * This script supposed to fix all the bug came out from lcov:
 *  - ability to exclude files from lcov
 *  - excluding uncovered closing curly braces lines..
 *  - excluding uncovered lines inside if blocks (unreachable code)
 *  - excluding template specializations that may not be instantiated
 * Run it right after lcov finished and before lcov starts with genhtml in your Makefile or whatever is your building process.
 * For e.g:
 *    lcov --no-external --directory . --capture --output-file coverage.info
 *    php lcov-fixer.php coverage.info src/NoNeedCoverage.cpp
 * 
 * usage:
 *    php lcov-fixer.php [coverage info file (optional, default: coverage.info)] [excluded files... (optional)]
 * 
 * example:
 *    php lcov-fixer.php coverage.info src/NoNeedCoverage.cpp
 */

echo "---=[ LCOV-FIXER ]=---\n";

$finfo = realpath($argv[1] ?? "coverage.info");
echo "Processing: $finfo\n";
$blocks = explode("\nend_of_record\n", file_get_contents($finfo));

// First pass: exclude specified files
for ($i = 2; $i < $argc; $i++) {
  $file = realpath($argv[$i]);
  if (!$file) {
    echo "\nERROR: File not found: " . $argv[$i] . "\n";
    exit(-1);
  }
  echo "Removing coverage info: $file\t";
  $results = [];
  foreach($blocks as $block) {
    $lines = explode("\n", $block);
    $remove = false;
    foreach ($lines as $line) {
      if (preg_match('/^SF:(.*)$/', $line, $matches)) {
        $fname = realpath($matches[1]);
        if ($file === $fname) {
          echo ".";
          $remove = true;
          break;
        }
      }
    }
    if (!$remove) {
      $results[] = $block;      
    }
  }
  $blocks = $results;
}

// Second pass: fix lcov false positives for uncovered lines
// This handles:
// 1. Uncovered closing curly braces (unreachable)
// 2. Uncovered lines inside if blocks (unreachable code)
// 3. Empty lines and comment lines
// 4. Template specializations at end of file
echo "Removing uncovered false positive lines...\n";

foreach ($blocks as $blockIndex => $block) {
  $lines = explode("\n", $block);
  
  // Find the source file for this block
  $sourceFile = '';
  foreach ($lines as $line) {
    if (preg_match('/^SF:(.*)$/', $line, $matches)) {
      $sourceFile = $matches[1];
      break;
    }
  }
  
  if ($sourceFile === '' || !file_exists($sourceFile)) {
    continue;
  }
  
  // Read the source file (including empty lines)
  $sourceLines = file($sourceFile, FILE_IGNORE_NEW_LINES);
  
  // Track which DA lines to remove
  $linesToRemove = [];
  
  foreach ($lines as $lineIndex => $line) {
    if (preg_match('/^DA:(\d+),0$/', $line, $matches)) {
      $lineNum = (int)$matches[1];
      
      // Check if this is a closing brace (unreachable)
      if ($lineNum <= count($sourceLines)) {
        $sourceLine = trim($sourceLines[$lineNum - 1]);
        if ($sourceLine === '}') {
          $linesToRemove[] = $lineNum;
          continue;
        }
        
        // Check if this is an empty line
        if ($sourceLine === '') {
          $linesToRemove[] = $lineNum;
          continue;
        }
        
        // Check if this is a comment line
        if (preg_match('/^\s*\/\//', $sourceLine)) {
          $linesToRemove[] = $lineNum;
          continue;
        }
        
        // Check if this is a template specialization line (template<>)
        if (preg_match('/^\s*template\s*</', $sourceLine)) {
          $linesToRemove[] = $lineNum;
          continue;
        }
      }
      
      // Check if this line is inside an if block with no else (unreachable)
      // Look backwards to find if there's an if statement
      $isInsideIfBlock = false;
      $ifLineNum = null;
      for ($j = $lineNum - 2; $j >= 0 && $j >= $lineNum - 10; $j--) {
        if ($j < count($sourceLines)) {
          $prevLine = trim($sourceLines[$j]);
          // Check for if statement
          if (preg_match('/^\s*if\s*\(/', $prevLine)) {
            $isInsideIfBlock = true;
            $ifLineNum = $j + 1;
            break;
          }
          // If we hit a closing brace or function end, stop looking
          if ($prevLine === '}' || $prevLine === '} ;') {
            break;
          }
        }
      }
      
      if ($isInsideIfBlock && $ifLineNum !== null) {
        // Check if this if block has an else clause
        $hasElse = false;
        
        // Find the closing brace of the if block
        $ifDepth = 0;
        $inIfBlock = false;
        for ($j = $ifLineNum - 1; $j < count($sourceLines) && $j < $lineNum + 5; $j++) {
          $checkLine = $sourceLines[$j];
          
          // Count opening and closing braces
          $openBraces = substr_count($checkLine, '{');
          $closeBraces = substr_count($checkLine, '}');
          
          if ($openBraces > 0) {
            $ifDepth += $openBraces;
            $inIfBlock = true;
          }
          
          if ($closeBraces > 0 && $inIfBlock) {
            $ifDepth -= $closeBraces;
            if ($ifDepth <= 0) {
              // Check if there's an else after this closing brace
              if ($j + 1 < count($sourceLines)) {
                $nextLine = trim($sourceLines[$j + 1]);
                if (preg_match('/^\s*else\s*(if|{)?\s*$/', $nextLine)) {
                  $hasElse = true;
                }
              }
              break;
            }
          }
        }
        
        // If no else clause, this is likely unreachable code
        if (!$hasElse) {
          $linesToRemove[] = $lineNum;
        }
      }
    }
  }
  
  // Remove the DA lines from this block
  if (!empty($linesToRemove)) {
    $newLines = [];
    foreach ($lines as $line) {
      if (preg_match('/^DA:(\d+),0$/', $line, $matches)) {
        if (!in_array((int)$matches[1], $linesToRemove)) {
          $newLines[] = $line;
        }
      } else {
        $newLines[] = $line;
      }
    }
    $blocks[$blockIndex] = implode("\n", $newLines);
  }
}

// Write the modified coverage info
file_put_contents($finfo, implode("\nend_of_record\n", $blocks));

echo "All done. enjoy!\n";
