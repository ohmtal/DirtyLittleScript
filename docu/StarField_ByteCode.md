# BaseFlux StarField ByteCode
Version: 2026-11-26

**NOTE**: op_constuct_array had only one parameter defined in dump so the following 
op_exit is not correct. It should be the second parameter.

## Analyse

A Post-Optimizer could do some work here. But when i remove bytes what happen to 
the jump offsets - they must be all recalculated. 

### So let's look what can be done at compile time

My parser will hate this, because the tape did not move - would be easier in a AST?!

This could be easy saved to globals on compile time  `caption = "StarField again with fields"`:
```
0000: [11]op_literal           [ 1861, 0, 0, 4 ] 
0005: [05]op_save_global       [ 17 ]
```


The same Value is commited 3 times in a row `flux::clearColor = { 20 20 20 255 }`:
```
0196: [11]op_literal           [ 20, 0, 0, 2 ]
0201: [11]op_literal           [ 20, 0, 0, 2 ]
0206: [11]op_literal           [ 20, 0, 0, 2 ]
```

Note I had added a Literal Stack why did I remove it again ?!
I guess fetching from stack was not faster as advance_value but there is a memcpy
so it should be. 

On CompileTime i could hold the last ByteCode and if it's the
same i could add a `op_dup`


This also looks not so cool:
```
0493: [43]op_cfunc             [ 16, 3 ]
0496: [05]op_save_global       [ 49 ]
0498: [04]op_load_global       [ 47 ]
0500: [04]op_load_global       [ 48 ]
0502: [04]op_load_global       [ 49 ]
0504: [04]op_load_global       [ 49 ]
0506: [43]op_cfunc             [ 74, 4 ]
```


## Code
```
===== TAPE DUMP =====
0000: [11]op_literal           [ 1861, 0, 0, 4 ] 
0005: [05]op_save_global       [ 17 ]
0007: [11]op_literal           [ 0, 1082654720, 0, 1 ]
0012: [05]op_save_global       [ 18 ]
0014: [11]op_literal           [ 1025, 0, 0, 2 ]
0019: [05]op_save_global       [ 19 ]
0021: [11]op_literal           [ 0, 1077608448, 0, 1 ]
0026: [05]op_save_global       [ 20 ]
0028: [11]op_literal           [ 327155712, 1104150528, 0, 1 ]
0033: [05]op_save_global       [ 21 ]
0035: [11]op_literal           [ 331350016, 1104150528, 0, 1 ]
0040: [05]op_save_global       [ 22 ]
0042: [11]op_literal           [ 335544320, 1104150528, 0, 1 ]
0047: [05]op_save_global       [ 23 ]
0049: [11]op_literal           [ 339738624, 1104150528, 0, 1 ]
0054: [05]op_save_global       [ 24 ]
0056: [11]op_literal           [ 343932928, 1104150528, 0, 1 ]
0061: [05]op_save_global       [ 25 ]
0063: [11]op_literal           [ 0, 1082392576, 0, 1 ]
0068: [05]op_save_global       [ 26 ]
0070: [11]op_literal           [ 0, 1081344000, 0, 1 ]
0075: [05]op_save_global       [ 27 ]
0077: [04]op_load_global       [ 26 ]
0079: [11]op_literal           [ 2, 0, 0, 2 ]
0084: [17]op_div
0085: [05]op_save_global       [ 28 ]
0087: [04]op_load_global       [ 28 ]
0089: [11]op_literal           [ 4294967295, 4294967295, 0, 2 ]
0094: [16]op_mul
0095: [05]op_save_global       [ 29 ]
0097: [04]op_load_global       [ 27 ]
0099: [11]op_literal           [ 2, 0, 0, 2 ]
0104: [17]op_div
0105: [05]op_save_global       [ 30 ]
0107: [04]op_load_global       [ 30 ]
0109: [11]op_literal           [ 4294967295, 4294967295, 0, 2 ]
0114: [16]op_mul
0115: [05]op_save_global       [ 31 ]
0117: [11]op_literal           [ 4200, 0, 0, 2 ]
0122: [05]op_save_global       [ 32 ]
0124: [11]op_literal           [ 0, 1067450368, 0, 1 ]
0129: [05]op_save_global       [ 33 ]
0131: [11]op_literal           [ 3539053052, 1062232653, 0, 1 ]
0136: [05]op_save_global       [ 34 ]
0138: [11]op_literal           [ 200, 0, 0, 2 ]
0143: [11]op_literal           [ 220, 0, 0, 2 ]
0148: [11]op_literal           [ 220, 0, 0, 2 ]
0153: [11]op_literal           [ 255, 0, 0, 2 ]
0158: [10]op_constuct_array    [ 4 ]
0160: [01]op_pop_stack
0161: [43]op_cfunc             [ 104, 1 ]
0164: [03]op_clear_temp_arrays
0165: [05]op_save_global       [ 35 ]
0167: [44]op_func_def          [ 171 ]
0169: [36]op_jump              [ 222 ]
0171: [11]op_literal           [ 0, 0, 0, 2 ]
0176: [05]op_save_global       [ 36 ]
0178: [11]op_literal           [ 69, 22018, 0, 4 ]
0183: [05]op_save_global       [ 37 ]
0185: [04]op_load_global       [ 17 ]
0187: [05]op_save_global       [ 38 ]
0189: [11]op_literal           [ 1862, 22018, 0, 4 ]
0194: [05]op_save_global       [ 39 ]
0196: [11]op_literal           [ 20, 0, 0, 2 ]
0201: [11]op_literal           [ 20, 0, 0, 2 ]
0206: [11]op_literal           [ 20, 0, 0, 2 ]
0211: [11]op_literal           [ 255, 0, 0, 2 ]
0216: [10]op_constuct_array    [ 4 ]
0218: [00]op_exit
0219: [05]op_save_global       [ 40 ]
0221: [45]op_return
0222: [05]op_save_global       [ 0 ]
0224: [44]op_func_def          [ 228 ]
0226: [36]op_jump              [ 266 ]
0228: [04]op_load_global       [ 32 ]
0230: [41]op_for_range_start   [ 41, 265 ]
0233: [04]op_load_global       [ 41 ]
0235: [04]op_load_global       [ 29 ]
0237: [04]op_load_global       [ 28 ]
0239: [43]op_cfunc             [ 4, 2 ]
0242: [04]op_load_global       [ 31 ]
0244: [04]op_load_global       [ 30 ]
0246: [43]op_cfunc             [ 4, 2 ]
0249: [43]op_cfunc             [ 3, 0 ]
0252: [10]op_constuct_array    [ 3 ]
0254: [00]op_exit
0255: [43]op_cfunc             [ 99, 1 ]
0258: [03]op_clear_temp_arrays
0259: [07]op_save_array        [ 42, 1 ]
0262: [42]op_for_range_jump    [ 41, 233 ]
0265: [45]op_return
0266: [05]op_save_global       [ 43 ]
0268: [44]op_func_def          [ 272 ]
0270: [36]op_jump              [ 287 ]
0272: [04]op_load_global       [ 26 ]
0274: [04]op_load_global       [ 27 ]
0276: [04]op_load_global       [ 26 ]
0278: [04]op_load_global       [ 27 ]
0280: [43]op_cfunc             [ 62, 4 ]
0283: [01]op_pop_stack
0284: [04]op_load_global       [ 43 ]
0286: [45]op_return
0287: [05]op_save_global       [ 1 ]
0289: [44]op_func_def          [ 293 ]
0291: [36]op_jump              [ 294 ]
0293: [45]op_return
0294: [05]op_save_global       [ 2 ]
0296: [43]op_cfunc             [ 96, 0 ]
0299: [05]op_save_global       [ 44 ]
0301: [44]op_func_def          [ 305 ]
0303: [36]op_jump              [ 554 ]
0305: [11]op_literal           [ 200, 0, 0, 2 ]
0310: [11]op_literal           [ 200, 0, 0, 2 ]
0315: [11]op_literal           [ 220, 0, 0, 2 ]
0320: [43]op_cfunc             [ 73, 3 ]
0323: [01]op_pop_stack
0324: [04]op_load_global       [ 32 ]
0326: [41]op_for_range_start   [ 41, 513 ]
0329: [04]op_load_global       [ 41 ]
0331: [06]op_load_array        [ 42, 1 ]
0334: [05]op_save_global       [ 45 ]
0336: [08]op_load_field        [ 45, 2147483650 ]
0339: [04]op_load_global       [ 34 ]
0341: [04]op_load_global       [ 15 ]
0343: [16]op_mul
0344: [15]op_sub
0345: [09]op_save_field        [ 45, 2147483650 ]
0348: [11]op_literal           [ 0, 1072693248, 0, 1 ]
0353: [08]op_load_field        [ 45, 2147483650 ]
0356: [17]op_div
0357: [05]op_save_global       [ 46 ]
0359: [04]op_load_global       [ 26 ]
0361: [11]op_literal           [ 0, 1071644672, 0, 1 ]
0366: [16]op_mul
0367: [08]op_load_field        [ 45, 2147483648 ]
0370: [04]op_load_global       [ 46 ]
0372: [16]op_mul
0373: [12]op_add
0374: [05]op_save_global       [ 47 ]
0376: [04]op_load_global       [ 27 ]
0378: [11]op_literal           [ 0, 1071644672, 0, 1 ]
0383: [16]op_mul
0384: [08]op_load_field        [ 45, 2147483649 ]
0387: [04]op_load_global       [ 46 ]
0389: [16]op_mul
0390: [12]op_add
0391: [05]op_save_global       [ 48 ]
0393: [08]op_load_field        [ 45, 2147483650 ]
0396: [11]op_literal           [ 0, 0, 0, 1 ]
0401: [31]op_lower
0402: [04]op_load_global       [ 47 ]
0404: [11]op_literal           [ 0, 0, 0, 1 ]
0409: [31]op_lower
0410: [28]op_or
0411: [04]op_load_global       [ 47 ]
0413: [04]op_load_global       [ 26 ]
0415: [30]op_greater
0416: [28]op_or
0417: [04]op_load_global       [ 48 ]
0419: [11]op_literal           [ 0, 0, 0, 1 ]
0424: [31]op_lower
0425: [28]op_or
0426: [04]op_load_global       [ 48 ]
0428: [04]op_load_global       [ 27 ]
0430: [30]op_greater
0431: [28]op_or
0432: [38]op_jump_if_false     [ 480 ]
0434: [04]op_load_global       [ 29 ]
0436: [04]op_load_global       [ 28 ]
0438: [43]op_cfunc             [ 4, 2 ]
0441: [09]op_save_field        [ 45, 2147483648 ]
0444: [04]op_load_global       [ 31 ]
0446: [04]op_load_global       [ 30 ]
0448: [43]op_cfunc             [ 4, 2 ]
0451: [09]op_save_field        [ 45, 2147483649 ]
0454: [11]op_literal           [ 0, 1072693248, 0, 1 ]
0459: [09]op_save_field        [ 45, 2147483650 ]
0462: [04]op_load_global       [ 26 ]
0464: [08]op_load_field        [ 45, 2147483648 ]
0467: [12]op_add
0468: [05]op_save_global       [ 47 ]
0470: [04]op_load_global       [ 27 ]
0472: [08]op_load_field        [ 45, 2147483649 ]
0475: [12]op_add
0476: [05]op_save_global       [ 48 ]
0478: [36]op_jump              [ 480 ]
0480: [11]op_literal           [ 0, 1073741824, 0, 1 ]
0485: [11]op_literal           [ 858993459, 1070805811, 0, 1 ]
0490: [08]op_load_field        [ 45, 2147483650 ]
0493: [43]op_cfunc             [ 16, 3 ]
0496: [05]op_save_global       [ 49 ]
0498: [04]op_load_global       [ 47 ]
0500: [04]op_load_global       [ 48 ]
0502: [04]op_load_global       [ 49 ]
0504: [04]op_load_global       [ 49 ]
0506: [43]op_cfunc             [ 74, 4 ]
0509: [01]op_pop_stack
0510: [42]op_for_range_jump    [ 41, 329 ]
0513: [11]op_literal           [ 10, 0, 0, 2 ]
0518: [11]op_literal           [ 30, 0, 0, 2 ]
0523: [11]op_literal           [ 1, 0, 0, 2 ]
0528: [43]op_cfunc             [ 69, 3 ]
0531: [01]op_pop_stack
0532: [11]op_literal           [ 10, 0, 0, 2 ]
0537: [11]op_literal           [ 60, 0, 0, 2 ]
0542: [04]op_load_global       [ 17 ]
0544: [11]op_literal           [ 0, 1073217536, 0, 1 ]
0549: [43]op_cfunc             [ 65, 4 ]
0552: [01]op_pop_stack
0553: [45]op_return
0554: [05]op_save_global       [ 3 ]
0556: [44]op_func_def          [ 560 ]
0558: [36]op_jump              [ 583 ]
0560: [04]op_load_global       [ 9 ]
0562: [04]op_load_global       [ 18 ]
0564: [32]op_equal
0565: [38]op_jump_if_false     [ 582 ]
0567: [04]op_load_global       [ 10 ]
0569: [04]op_load_global       [ 20 ]
0571: [32]op_equal
0572: [38]op_jump_if_false     [ 580 ]
0574: [43]op_cfunc             [ 64, 0 ]
0577: [01]op_pop_stack
0578: [36]op_jump              [ 580 ]
0580: [36]op_jump              [ 582 ]
0582: [45]op_return
0583: [05]op_save_global       [ 4 ]
0585: [44]op_func_def          [ 589 ]
0587: [36]op_jump              [ 615 ]
0589: [04]op_load_global       [ 9 ]
0591: [04]op_load_global       [ 19 ]
0593: [32]op_equal
0594: [38]op_jump_if_false     [ 614 ]
0596: [11]op_literal           [ 75, 22018, 0, 4 ]
0601: [04]op_load_global       [ 12 ]
0603: [11]op_literal           [ 76, 22018, 0, 4 ]
0608: [43]op_cfunc             [ 54, 3 ]
0611: [01]op_pop_stack
0612: [36]op_jump              [ 614 ]
0614: [45]op_return
0615: [05]op_save_global       [ 6 ]
0617: [00]op_exit
=====================
```

## op_literal optimized with op_dup and using Literal Register:

Saved 151 Slots = 604 bytes in code - but as expected not faster ;)

```
===== TAPE DUMP =====
0000: [12]op_literal           [ 39 ]
0002: [06]op_save_global       [ 17 ]
0004: [12]op_literal           [ 1 ]
0006: [06]op_save_global       [ 18 ]
0008: [12]op_literal           [ 2 ]
0010: [06]op_save_global       [ 19 ]
0012: [12]op_literal           [ 4 ]
0014: [06]op_save_global       [ 20 ]
0016: [12]op_literal           [ 5 ]
0018: [06]op_save_global       [ 21 ]
0020: [12]op_literal           [ 6 ]
0022: [06]op_save_global       [ 22 ]
0024: [12]op_literal           [ 7 ]
0026: [06]op_save_global       [ 23 ]
0028: [12]op_literal           [ 8 ]
0030: [06]op_save_global       [ 24 ]
0032: [12]op_literal           [ 9 ]
0034: [06]op_save_global       [ 25 ]
0036: [12]op_literal           [ 40 ]
0038: [06]op_save_global       [ 26 ]
0040: [12]op_literal           [ 41 ]
0042: [06]op_save_global       [ 27 ]
0044: [05]op_load_global       [ 26 ]
0046: [12]op_literal           [ 30 ]
0048: [18]op_div              
0049: [06]op_save_global       [ 28 ]
0051: [05]op_load_global       [ 28 ]
0053: [12]op_literal           [ 42 ]
0055: [17]op_mul              
0056: [06]op_save_global       [ 29 ]
0058: [05]op_load_global       [ 27 ]
0060: [12]op_literal           [ 30 ]
0062: [18]op_div              
0063: [06]op_save_global       [ 30 ]
0065: [05]op_load_global       [ 30 ]
0067: [12]op_literal           [ 42 ]
0069: [17]op_mul              
0070: [06]op_save_global       [ 31 ]
0072: [12]op_literal           [ 43 ]
0074: [06]op_save_global       [ 32 ]
0076: [12]op_literal           [ 44 ]
0078: [06]op_save_global       [ 33 ]
0080: [12]op_literal           [ 45 ]
0082: [06]op_save_global       [ 34 ]
0084: [12]op_literal           [ 46 ]

0086: [12]op_literal           [ 47 ]
0088: [03]op_dup              

0089: [12]op_literal           [ 14 ]
0091: [11]op_constuct_array    [ 4 ]
0093: [01]op_pop_stack        
0094: [44]op_cfunc             [ 104, 1 ]
0097: [04]op_clear_temp_arrays
0098: [06]op_save_global       [ 35 ]
0100: [45]op_func_def          [ 104 ]
0102: [37]op_jump              [ 132 ]
0104: [12]op_literal           [ 13 ]
0106: [06]op_save_global       [ 36 ]
0108: [12]op_literal           [ 17 ]
0110: [06]op_save_global       [ 37 ]
0112: [05]op_load_global       [ 17 ]
0114: [06]op_save_global       [ 38 ]
0116: [12]op_literal           [ 48 ]
0118: [06]op_save_global       [ 39 ]

0120: [12]op_literal           [ 49 ]
0122: [03]op_dup              
0123: [03]op_dup              

0124: [12]op_literal           [ 14 ]
0126: [11]op_constuct_array    [ 4 ]
0128: [00]op_exit             
0129: [06]op_save_global       [ 40 ]
0131: [46]op_return           
0132: [06]op_save_global       [ 0 ]
0134: [45]op_func_def          [ 138 ]
0136: [37]op_jump              [ 176 ]
0138: [05]op_load_global       [ 32 ]
0140: [42]op_for_range_start   [ 41, 175 ]
0143: [05]op_load_global       [ 41 ]
0145: [05]op_load_global       [ 29 ]
0147: [05]op_load_global       [ 28 ]
0149: [44]op_cfunc             [ 4, 2 ]
0152: [05]op_load_global       [ 31 ]
0154: [05]op_load_global       [ 30 ]
0156: [44]op_cfunc             [ 4, 2 ]
0159: [44]op_cfunc             [ 3, 0 ]
0162: [11]op_constuct_array    [ 3 ]
0164: [00]op_exit             
0165: [44]op_cfunc             [ 99, 1 ]
0168: [04]op_clear_temp_arrays
0169: [08]op_save_array        [ 42, 1 ]
0172: [43]op_for_range_jump    [ 41, 143 ]
0175: [46]op_return           
0176: [06]op_save_global       [ 43 ]
0178: [45]op_func_def          [ 182 ]
0180: [37]op_jump              [ 197 ]
0182: [05]op_load_global       [ 26 ]
0184: [05]op_load_global       [ 27 ]
0186: [05]op_load_global       [ 26 ]
0188: [05]op_load_global       [ 27 ]
0190: [44]op_cfunc             [ 62, 4 ]
0193: [01]op_pop_stack        
0194: [05]op_load_global       [ 43 ]
0196: [46]op_return           
0197: [06]op_save_global       [ 1 ]
0199: [45]op_func_def          [ 203 ]
0201: [37]op_jump              [ 204 ]
0203: [46]op_return           
0204: [06]op_save_global       [ 2 ]
0206: [44]op_cfunc             [ 96, 0 ]
0209: [06]op_save_global       [ 44 ]
0211: [45]op_func_def          [ 215 ]
0213: [37]op_jump              [ 409 ]

0215: [12]op_literal           [ 46 ]
0217: [03]op_dup              

0218: [12]op_literal           [ 47 ]
0220: [44]op_cfunc             [ 73, 3 ]
0223: [01]op_pop_stack        
0224: [05]op_load_global       [ 32 ]
0226: [42]op_for_range_start   [ 41, 386 ]
---- HOT PART >>>>>>>>
0229: [05]op_load_global       [ 41 ]
0231: [07]op_load_array        [ 42, 1 ]
0234: [06]op_save_global       [ 45 ]
0236: [09]op_load_field        [ 45, 2147483650 ]
0239: [05]op_load_global       [ 34 ]
0241: [05]op_load_global       [ 15 ]
0243: [17]op_mul              
0244: [16]op_sub              
0245: [10]op_save_field        [ 45, 2147483650 ]
0248: [12]op_literal           [ 24 ]
0250: [09]op_load_field        [ 45, 2147483650 ]
0253: [18]op_div              
0254: [06]op_save_global       [ 46 ]
0256: [05]op_load_global       [ 26 ]
0258: [12]op_literal           [ 50 ]
0260: [17]op_mul              
0261: [09]op_load_field        [ 45, 2147483648 ]
0264: [05]op_load_global       [ 46 ]
0266: [17]op_mul              
0267: [13]op_add              
0268: [06]op_save_global       [ 47 ]
0270: [05]op_load_global       [ 27 ]
0272: [12]op_literal           [ 50 ]
0274: [17]op_mul              
0275: [09]op_load_field        [ 45, 2147483649 ]
0278: [05]op_load_global       [ 46 ]
0280: [17]op_mul              
0281: [13]op_add              
0282: [06]op_save_global       [ 48 ]
0284: [09]op_load_field        [ 45, 2147483650 ]
0287: [12]op_literal           [ 27 ]
0289: [32]op_lower            
0290: [05]op_load_global       [ 47 ]
0292: [12]op_literal           [ 27 ]
0294: [32]op_lower            
0295: [29]op_or               
0296: [05]op_load_global       [ 47 ]
0298: [05]op_load_global       [ 26 ]
0300: [31]op_greater          
0301: [29]op_or               
0302: [05]op_load_global       [ 48 ]
0304: [12]op_literal           [ 27 ]
0306: [32]op_lower            
0307: [29]op_or               
0308: [05]op_load_global       [ 48 ]
0310: [05]op_load_global       [ 27 ]
0312: [31]op_greater          
0313: [29]op_or               
0314: [39]op_jump_if_false     [ 359 ]
0316: [05]op_load_global       [ 29 ]
0318: [05]op_load_global       [ 28 ]
0320: [44]op_cfunc             [ 4, 2 ]
0323: [10]op_save_field        [ 45, 2147483648 ]
0326: [05]op_load_global       [ 31 ]
0328: [05]op_load_global       [ 30 ]
0330: [44]op_cfunc             [ 4, 2 ]
0333: [10]op_save_field        [ 45, 2147483649 ]
0336: [12]op_literal           [ 24 ]
0338: [10]op_save_field        [ 45, 2147483650 ]
0341: [05]op_load_global       [ 26 ]
0343: [09]op_load_field        [ 45, 2147483648 ]
0346: [13]op_add              
0347: [06]op_save_global       [ 47 ]
0349: [05]op_load_global       [ 27 ]
0351: [09]op_load_field        [ 45, 2147483649 ]
0354: [13]op_add              
0355: [06]op_save_global       [ 48 ]
0357: [37]op_jump              [ 359 ]
0359: [12]op_literal           [ 23 ]
0361: [12]op_literal           [ 51 ]
0363: [09]op_load_field        [ 45, 2147483650 ]
0366: [44]op_cfunc             [ 16, 3 ]
0369: [06]op_save_global       [ 49 ]

0371: [05]op_load_global       [ 47 ]
0373: [05]op_load_global       [ 48 ]
0375: [05]op_load_global       [ 49 ]
0377: [05]op_load_global       [ 49 ]
0379: [44]op_cfunc             [ 74, 4 ]

0382: [01]op_pop_stack        
<<<<<< HOT PART -----
0383: [43]op_for_range_jump    [ 41, 229 ]
0386: [12]op_literal           [ 29 ]
0388: [12]op_literal           [ 20 ]
0390: [12]op_literal           [ 25 ]
0392: [44]op_cfunc             [ 69, 3 ]
0395: [01]op_pop_stack        
0396: [12]op_literal           [ 29 ]
0398: [12]op_literal           [ 34 ]
0400: [05]op_load_global       [ 17 ]
0402: [12]op_literal           [ 52 ]
0404: [44]op_cfunc             [ 65, 4 ]
0407: [01]op_pop_stack        
0408: [46]op_return           
0409: [06]op_save_global       [ 3 ]
0411: [45]op_func_def          [ 415 ]
0413: [37]op_jump              [ 438 ]
0415: [05]op_load_global       [ 9 ]
0417: [05]op_load_global       [ 18 ]
0419: [33]op_equal            
0420: [39]op_jump_if_false     [ 437 ]
0422: [05]op_load_global       [ 10 ]
0424: [05]op_load_global       [ 20 ]
0426: [33]op_equal            
0427: [39]op_jump_if_false     [ 435 ]
0429: [44]op_cfunc             [ 64, 0 ]
0432: [01]op_pop_stack        
0433: [37]op_jump              [ 435 ]
0435: [37]op_jump              [ 437 ]
0437: [46]op_return           
0438: [06]op_save_global       [ 4 ]
0440: [45]op_func_def          [ 444 ]
0442: [37]op_jump              [ 464 ]
0444: [05]op_load_global       [ 9 ]
0446: [05]op_load_global       [ 19 ]
0448: [33]op_equal            
0449: [39]op_jump_if_false     [ 463 ]
0451: [12]op_literal           [ 36 ]
0453: [05]op_load_global       [ 12 ]
0455: [12]op_literal           [ 37 ]
0457: [44]op_cfunc             [ 54, 3 ]
0460: [01]op_pop_stack        
0461: [37]op_jump              [ 463 ]
0463: [46]op_return           
0464: [06]op_save_global       [ 6 ]
0466: [00]op_exit             
=====================
```

# op_load_global optimized with op_dup:

does not really speed up .

```
===== TAPE DUMP =====
0000: [12]op_literal           [ 39 ]
0002: [06]op_save_global       [ 17 ]
0004: [12]op_literal           [ 1 ]
0006: [06]op_save_global       [ 18 ]
0008: [12]op_literal           [ 2 ]
0010: [06]op_save_global       [ 19 ]
0012: [12]op_literal           [ 4 ]
0014: [06]op_save_global       [ 20 ]
0016: [12]op_literal           [ 5 ]
0018: [06]op_save_global       [ 21 ]
0020: [12]op_literal           [ 6 ]
0022: [06]op_save_global       [ 22 ]
0024: [12]op_literal           [ 7 ]
0026: [06]op_save_global       [ 23 ]
0028: [12]op_literal           [ 8 ]
0030: [06]op_save_global       [ 24 ]
0032: [12]op_literal           [ 9 ]
0034: [06]op_save_global       [ 25 ]
0036: [12]op_literal           [ 40 ]
0038: [06]op_save_global       [ 26 ]
0040: [12]op_literal           [ 41 ]
0042: [06]op_save_global       [ 27 ]
0044: [05]op_load_global       [ 26 ]
0046: [12]op_literal           [ 30 ]
0048: [18]op_div              
0049: [06]op_save_global       [ 28 ]
0051: [05]op_load_global       [ 28 ]
0053: [12]op_literal           [ 42 ]
0055: [17]op_mul              
0056: [06]op_save_global       [ 29 ]
0058: [05]op_load_global       [ 27 ]
0060: [12]op_literal           [ 30 ]
0062: [18]op_div              
0063: [06]op_save_global       [ 30 ]
0065: [05]op_load_global       [ 30 ]
0067: [12]op_literal           [ 42 ]
0069: [17]op_mul              
0070: [06]op_save_global       [ 31 ]
0072: [12]op_literal           [ 43 ]
0074: [06]op_save_global       [ 32 ]
0076: [12]op_literal           [ 44 ]
0078: [06]op_save_global       [ 33 ]
0080: [12]op_literal           [ 45 ]
0082: [06]op_save_global       [ 34 ]
0084: [12]op_literal           [ 46 ]
0086: [12]op_literal           [ 47 ]
0088: [03]op_dup              
0089: [12]op_literal           [ 14 ]
0091: [11]op_constuct_array    [ 4 ]
0093: [01]op_pop_stack        
0094: [44]op_cfunc             [ 104, 1 ]
0097: [04]op_clear_temp_arrays
0098: [06]op_save_global       [ 35 ]
0100: [45]op_func_def          [ 104 ]
0102: [37]op_jump              [ 132 ]
0104: [12]op_literal           [ 13 ]
0106: [06]op_save_global       [ 36 ]
0108: [12]op_literal           [ 17 ]
0110: [06]op_save_global       [ 37 ]
0112: [05]op_load_global       [ 17 ]
0114: [06]op_save_global       [ 38 ]
0116: [12]op_literal           [ 48 ]
0118: [06]op_save_global       [ 39 ]
0120: [12]op_literal           [ 49 ]
0122: [03]op_dup              
0123: [03]op_dup              
0124: [12]op_literal           [ 14 ]
0126: [11]op_constuct_array    [ 4 ]
0128: [00]op_exit             
0129: [06]op_save_global       [ 40 ]
0131: [46]op_return           
0132: [06]op_save_global       [ 0 ]
0134: [45]op_func_def          [ 138 ]
0136: [37]op_jump              [ 176 ]
0138: [05]op_load_global       [ 32 ]
0140: [42]op_for_range_start   [ 41, 175 ]
0143: [05]op_load_global       [ 41 ]
0145: [05]op_load_global       [ 29 ]
0147: [05]op_load_global       [ 28 ]
0149: [44]op_cfunc             [ 4, 2 ]
0152: [05]op_load_global       [ 31 ]
0154: [05]op_load_global       [ 30 ]
0156: [44]op_cfunc             [ 4, 2 ]
0159: [44]op_cfunc             [ 3, 0 ]
0162: [11]op_constuct_array    [ 3 ]
0164: [00]op_exit             
0165: [44]op_cfunc             [ 99, 1 ]
0168: [04]op_clear_temp_arrays
0169: [08]op_save_array        [ 42, 1 ]
0172: [43]op_for_range_jump    [ 41, 143 ]
0175: [46]op_return           
0176: [06]op_save_global       [ 43 ]
0178: [45]op_func_def          [ 182 ]
0180: [37]op_jump              [ 197 ]
0182: [05]op_load_global       [ 26 ]
0184: [05]op_load_global       [ 27 ]
0186: [05]op_load_global       [ 26 ]
0188: [05]op_load_global       [ 27 ]
0190: [44]op_cfunc             [ 62, 4 ]
0193: [01]op_pop_stack        
0194: [05]op_load_global       [ 43 ]
0196: [46]op_return           
0197: [06]op_save_global       [ 1 ]
0199: [45]op_func_def          [ 203 ]
0201: [37]op_jump              [ 204 ]
0203: [46]op_return           
0204: [06]op_save_global       [ 2 ]
0206: [44]op_cfunc             [ 96, 0 ]
0209: [06]op_save_global       [ 44 ]
0211: [45]op_func_def          [ 215 ]
0213: [37]op_jump              [ 408 ]
0215: [12]op_literal           [ 46 ]
0217: [03]op_dup              
0218: [12]op_literal           [ 47 ]
0220: [44]op_cfunc             [ 73, 3 ]
0223: [01]op_pop_stack        
0224: [05]op_load_global       [ 32 ]
0226: [42]op_for_range_start   [ 41, 385 ]
0229: [05]op_load_global       [ 41 ]
0231: [07]op_load_array        [ 42, 1 ]
0234: [06]op_save_global       [ 45 ]
0236: [09]op_load_field        [ 45, 2147483650 ]
0239: [05]op_load_global       [ 34 ]
0241: [05]op_load_global       [ 15 ]
0243: [17]op_mul              
0244: [16]op_sub              
0245: [10]op_save_field        [ 45, 2147483650 ]
0248: [12]op_literal           [ 24 ]
0250: [09]op_load_field        [ 45, 2147483650 ]
0253: [18]op_div              
0254: [06]op_save_global       [ 46 ]
0256: [05]op_load_global       [ 26 ]
0258: [12]op_literal           [ 50 ]
0260: [17]op_mul              
0261: [09]op_load_field        [ 45, 2147483648 ]
0264: [05]op_load_global       [ 46 ]
0266: [17]op_mul              
0267: [13]op_add              
0268: [06]op_save_global       [ 47 ]
0270: [05]op_load_global       [ 27 ]
0272: [12]op_literal           [ 50 ]
0274: [17]op_mul              
0275: [09]op_load_field        [ 45, 2147483649 ]
0278: [05]op_load_global       [ 46 ]
0280: [17]op_mul              
0281: [13]op_add              
0282: [06]op_save_global       [ 48 ]
0284: [09]op_load_field        [ 45, 2147483650 ]
0287: [12]op_literal           [ 27 ]
0289: [32]op_lower            
0290: [05]op_load_global       [ 47 ]
0292: [12]op_literal           [ 27 ]
0294: [32]op_lower            
0295: [29]op_or               
0296: [05]op_load_global       [ 47 ]
0298: [05]op_load_global       [ 26 ]
0300: [31]op_greater          
0301: [29]op_or               
0302: [05]op_load_global       [ 48 ]
0304: [12]op_literal           [ 27 ]
0306: [32]op_lower            
0307: [29]op_or               
0308: [05]op_load_global       [ 48 ]
0310: [05]op_load_global       [ 27 ]
0312: [31]op_greater          
0313: [29]op_or               
0314: [39]op_jump_if_false     [ 359 ]
0316: [05]op_load_global       [ 29 ]
0318: [05]op_load_global       [ 28 ]
0320: [44]op_cfunc             [ 4, 2 ]
0323: [10]op_save_field        [ 45, 2147483648 ]
0326: [05]op_load_global       [ 31 ]
0328: [05]op_load_global       [ 30 ]
0330: [44]op_cfunc             [ 4, 2 ]
0333: [10]op_save_field        [ 45, 2147483649 ]
0336: [12]op_literal           [ 24 ]
0338: [10]op_save_field        [ 45, 2147483650 ]
0341: [05]op_load_global       [ 26 ]
0343: [09]op_load_field        [ 45, 2147483648 ]
0346: [13]op_add              
0347: [06]op_save_global       [ 47 ]
0349: [05]op_load_global       [ 27 ]
0351: [09]op_load_field        [ 45, 2147483649 ]
0354: [13]op_add              
0355: [06]op_save_global       [ 48 ]
0357: [37]op_jump              [ 359 ]
0359: [12]op_literal           [ 23 ]
0361: [12]op_literal           [ 51 ]
0363: [09]op_load_field        [ 45, 2147483650 ]
0366: [44]op_cfunc             [ 16, 3 ]
0369: [06]op_save_global       [ 49 ]
0371: [05]op_load_global       [ 47 ]
0373: [05]op_load_global       [ 48 ]
0375: [05]op_load_global       [ 49 ]
0377: [03]op_dup                                <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 
0378: [44]op_cfunc             [ 74, 4 ]
0381: [01]op_pop_stack        
0382: [43]op_for_range_jump    [ 41, 229 ]
0385: [12]op_literal           [ 29 ]
0387: [12]op_literal           [ 20 ]
0389: [12]op_literal           [ 25 ]
0391: [44]op_cfunc             [ 69, 3 ]
0394: [01]op_pop_stack        
0395: [12]op_literal           [ 29 ]
0397: [12]op_literal           [ 34 ]
0399: [05]op_load_global       [ 17 ]
0401: [12]op_literal           [ 52 ]
0403: [44]op_cfunc             [ 65, 4 ]
0406: [01]op_pop_stack        
0407: [46]op_return           
0408: [06]op_save_global       [ 3 ]
0410: [45]op_func_def          [ 414 ]
0412: [37]op_jump              [ 437 ]
0414: [05]op_load_global       [ 9 ]
0416: [05]op_load_global       [ 18 ]
0418: [33]op_equal            
0419: [39]op_jump_if_false     [ 436 ]
0421: [05]op_load_global       [ 10 ]
0423: [05]op_load_global       [ 20 ]
0425: [33]op_equal            
0426: [39]op_jump_if_false     [ 434 ]
0428: [44]op_cfunc             [ 64, 0 ]
0431: [01]op_pop_stack        
0432: [37]op_jump              [ 434 ]
0434: [37]op_jump              [ 436 ]
0436: [46]op_return           
0437: [06]op_save_global       [ 4 ]
0439: [45]op_func_def          [ 443 ]
0441: [37]op_jump              [ 463 ]
0443: [05]op_load_global       [ 9 ]
0445: [05]op_load_global       [ 19 ]
0447: [33]op_equal            
0448: [39]op_jump_if_false     [ 462 ]
0450: [12]op_literal           [ 36 ]
0452: [05]op_load_global       [ 12 ]
0454: [12]op_literal           [ 37 ]
0456: [44]op_cfunc             [ 54, 3 ]
0459: [01]op_pop_stack        
0460: [37]op_jump              [ 462 ]
0462: [46]op_return           
0463: [06]op_save_global       [ 6 ]
0465: [00]op_exit             
=====================
```
