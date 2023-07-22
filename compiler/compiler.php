<?php

define('RAM', 1024);

$opcodes = [
    'NOP' => 0x00,
    'LDI' => 0x01,
    'LDA' => 0x02,
    'TAB' => 0x03,
    'ADD' => 0x04,
    'SUB' => 0x05,
    'STA' => 0x06,
    'RCH' => 0x07,
    'LPC' => 0x08,
    'INC' => 0x09,
    'DCR' => 0x0a,
    'CMP' => 0x0b,
    'JMP' => 0x0c,
    'DBG' => 0x0d,
    'IN'  => 0x0e,
    'OUT' => 0x0f,
    'BIT' => 0x10,
    'AND' => 0x11,
    'OR'  => 0x12,
    'XOR' => 0x13,
    'NOT' => 0x14,
    'SHL' => 0x15,
    'SHR' => 0x16,
    'CLS' => 0x17,
    'SDL' => 0x18,
    'SDR' => 0x19,
    'CRS' => 0x1a,
    'NCR' => 0x1b,
    'UDG' => 0x1c,
    'SPR' => 0x1d,
    'POS' => 0x1e,
    'DLY' => 0x1f,
    'RND' => 0x20,
    'PSH' => 0x21,
    'POP' => 0x22,
    'SBR' => 0x23,
    'RET' => 0x24,
    'NUM' => 0x25,
    'INM' => 0x26,
    'DCM' => 0x27,
    'SER' => 0x28
];

$labels = [];

if ( php_sapi_name() != 'cli' ) {
	die("Ushbu faylni faqatgina terminal orqali ishga tushirish mumkin!" . PHP_EOL);	
}

if ( empty( $_SERVER['argv'][1] ) ) {
	die("Mashina kodiga o'g'irish uchun fayl nomi kiritlmadi. " . basename(__FILE__) . " fayl.asm" . PHP_EOL);	
}

$filename = $_SERVER['argv'][1];

if ( file_exists( $filename ) ) {
	$src = explode( PHP_EOL, strtoupper(
		@file_get_contents( $filename )
	));

	if ( !empty( $src ) ) {
		$program = [];
    	$byte_count = -1;

    	foreach ($src as $line) {
    		if ( str_contains($line, "%DEFINE") ) {
    			if ( substr(trim( $line ), 0, 1) == ';' ) continue;

    			$defined_data = explode(";", $line);
    			if ( !empty( $defined_data[0] ) ) {
    				if ( str_contains( $defined_data[0], "BYTE" ) || str_contains( $defined_data[0], "WORD" ) ) {
    					die( '"BYTE" yoki "WORD" %define ning bir qismi bo\'la olmaydi!' );
    				}
    				
    				$defined_data_arguments = explode(" ", trim( $defined_data[0] ) );
    				if ( count( $defined_data_arguments ) == 3 ) {
    					if ( !intval($defined_data_arguments[1], 16) ) {
    						$labels[ $defined_data_arguments[1] ] = str_replace("X", "x", $defined_data_arguments[2]);
    						continue;

    					}else{
    						die( $defined_data_arguments[1] . ' yorlig\'i HEX qiymati sifatida qabul qilinadi, boshqa nomdan foydalaning!' );
    					}
    				}else{
    					die( '%define o\'z ichiga 2 ta argumentni oladi, lekin ' . ( count( $defined_data_arguments ) - 1 ). ' dona argument berilgan!' );
    				}
    			}
    		}

    		if ( !empty( $line ) ) {
    			$line_data = explode(";", $line);
    			if ( !empty( $line_data[1] ) && str_contains( $line_data[1], ":" ) ) {
    				die( "Izohlar uchun : belgisidan foydalanish man etiladi!" );
    			}

    			$line_data[0] = trim( $line_data[0] );
    			
    			if ( empty( $line_data[0] ) ) {
    				continue;
    			}

    			$line_data_arr = explode( ' ', $line_data[0] );
    			$byte_count += count( $line_data_arr );
    			
    			if ( !empty( $line_data_arr[1] ) &&  ( str_contains( $line_data_arr[1], '0X00' ) && strlen( $line_data_arr[1] ) == 6 ) ) {
    				$byte_count += 1;
    			}

    			if ( !empty( $line_data_arr[1] ) && preg_match('/^[a-zA-Z]$/', substr($line_data_arr[1], 0, 1) ) ) {
    				if ( in_array( ( explode( " " , $line_data[0])[0] ) , ['LDA', 'STA', 'LPC', 'JMP', 'SBR', 'NUM', 'INM', 'DCM']) ) {
    					$byte_count += 1;
    				}
    			}

    			if ( str_contains( $line_data[0], ":" ) ) {
    				if ( !empty( trim( explode( ":" , $line_data[0] )[1] ) ) ) {
    					die( 'Instruksiya yorliq bilan bir qatorda bo\'lishishi mumkin emas!' );
    				}

    				if ( in_array( substr($line_data[0], 0, -1) , [ 'BYTE', 'WORD' ] ) ) {
    					die( 'BYTE va WORD so\'zlari zaxiralangangani sabab yorlig\' nomi uchun foydalanib bo\'lmaydi!' );
    				}

    				$labels[ substr($line_data[0], 0, -1) ] = sprintf("0x%04x", intval($byte_count, 16));
                    $byte_count -= 1;
    			}

    			if ( str_contains( $line_data[0], "BYTE" ) || str_contains( $line_data[0], "WORD" ) ) $byte_count -= 1;
    		}
    	}

    	foreach ($src as $line) {
			if ( str_contains( $line, '%DEFINE' ) ) continue;

    		if ( !empty( $line ) ) {
    			if ( str_contains( $line, ":" ) ) continue;
				if ( substr(trim( $line ), 0, 1) == ';' ) continue;
				
				$line_data = explode( " ", trim( $line ) );
				$opcode = $line_data[0];
				
				if ( empty( $opcode ) ) continue;

				if ( array_key_exists( $opcode, $opcodes ) ) {
					$program[] = $opcodes[$opcode]; 
				}else{
					if ( $opcode != 'BYTE' && $opcode != 'WORD' ) {
						die("Aniqlanmagan opkod: $x{$line}");
					}
				}

				if ( isset($line_data[1]) && $line_data[1] != '' ) {
					$arg = $line_data[1];

					if ( str_contains($arg, 'Ox00') && strlen($arg) == 6 ) {
						$program[] = 0;
					}

					if ( preg_match('/^(?:0x)?[a-f0-9]{1,}$/i', $arg) ) {
						$value = intval($arg, 16);
					}else{	
						if ( isset($labels[$arg]) ) {
							if (str_contains( $labels[ $arg ], '0x00' ) && strlen( $labels[ $arg ] ) == 6 ) {
								if ( in_array($opcode, ['LDA', 'STA', 'LPC', 'JMP', 'SBR', 'NUM', 'INM', 'DCM']) ) {
									$program[] = 0;
								}
							}

							$value = intval($labels[ $arg ], 16);
						}else{
							die('Yorlig\' topilmadi: ' . $arg);
						}
					}

					if ( $value > 0xff ) {
						$program[] = $value >> 8;
						$program[] = $value & 0x00FF;
					}else{
						$program[] = $value;
					}
				}
    		}
    	}

    	if ( count( $program ) > RAM ) {
    		die( 'Dastur hajmi chegaradan oshib ketdi: ' . ( count( $program ) - RAM ) . ' bayt!' );
    	}

    	echo "Dastur uzunligi: ";
    	echo count( $program );
    	echo " bayt" . PHP_EOL;

    	echo str_repeat('-', 40) . PHP_EOL;

    	echo "Dastur yorlig'lari: ";
    	echo json_encode( $labels, JSON_PRETTY_PRINT ) . PHP_EOL;

    	echo str_repeat('-', 40) . PHP_EOL;

    	
    	echo "Dastur: ";
    	
    	echo strtoupper(
    		str_replace('0x', '', implode(
    			'', 
    			array_map(function( $x ){
					return sprintf("0x%02x", $x);
    			}, $program)
    		))
    	);

    	echo PHP_EOL;

    	echo "Dastur: " . PHP_EOL;
    	
    	$x = 0;
    	$xx = 0;

    	echo sprintf("0x%04x: ", dechex($xx));

    	foreach ($program as $section) {
    		if ($x == 4) {
    			$x = 0;
    			echo PHP_EOL;
    			echo sprintf("0x%04x: ", intval($xx, 16));
    		}
    		echo strtoupper( str_replace('0x', '', sprintf("0x%02x ", $section) ) );
    		$xx  += 1;
    		$x++;	
    	}

    	echo PHP_EOL;

    	echo str_repeat('-', 40) . PHP_EOL;

	}else{
		die("Fayl kontentini o'qishda xatolik!");
	}	
}else{
	die("Faylni ochishda xatolik!");
}