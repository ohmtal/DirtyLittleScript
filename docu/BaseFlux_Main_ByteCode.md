# BaseFlux main ByteCode
Version: 2026-11-26.2

## Analyse

op_pop_stack: after a c_func i need to emit this when no parameter is requested. 
This should be faster than a emitted value + if but i did not test it. 
Solution: op_cfunc_void! :D

```
[info] Welcome to BaseFlux DLS Console!
===== TAPE DUMP =====
0000: [12]op_literal           [ 0 ]
0002: [06]op_save_global       [ 17 ]
0004: [12]op_literal           [ 1 ]
0006: [06]op_save_global       [ 18 ]
0008: [12]op_literal           [ 2 ]
0010: [06]op_save_global       [ 19 ]
0012: [12]op_literal           [ 3 ]
0014: [06]op_save_global       [ 20 ]
0016: [12]op_literal           [ 4 ]
0018: [06]op_save_global       [ 21 ]
0020: [12]op_literal           [ 5 ]
0022: [06]op_save_global       [ 22 ]
0024: [12]op_literal           [ 6 ]
0026: [06]op_save_global       [ 23 ]
0028: [12]op_literal           [ 7 ]
0030: [06]op_save_global       [ 24 ]
0032: [12]op_literal           [ 8 ]
0034: [06]op_save_global       [ 25 ]
0036: [12]op_literal           [ 9 ]
0038: [06]op_save_global       [ 26 ]
0040: [12]op_literal           [ 10 ]
0042: [06]op_save_global       [ 27 ]
0044: [12]op_literal           [ 11 ]
0046: [06]op_save_global       [ 28 ]
0048: [12]op_literal           [ 12 ]
0050: [06]op_save_global       [ 29 ]
0052: [12]op_literal           [ 13 ]
0054: [12]op_literal           [ 14 ]
0056: [03]op_dup              
0057: [11]op_constuct_array    [ 3, 0 ]
0060: [06]op_save_global       [ 30 ]
0062: [12]op_literal           [ 15 ]
0064: [06]op_save_global       [ 31 ]
0066: [12]op_literal           [ 16 ]
0068: [06]op_save_global       [ 32 ]
0070: [45]op_func_def          [ 74 ]
0072: [37]op_jump              [ 103 ]
0074: [12]op_literal           [ 13 ]
0076: [06]op_save_global       [ 33 ]
0078: [12]op_literal           [ 17 ]
0080: [06]op_save_global       [ 34 ]
0082: [12]op_literal           [ 18 ]
0084: [06]op_save_global       [ 35 ]
0086: [12]op_literal           [ 19 ]
0088: [06]op_save_global       [ 36 ]
0090: [12]op_literal           [ 20 ]
0092: [12]op_literal           [ 15 ]
0094: [03]op_dup              
0095: [12]op_literal           [ 14 ]
0097: [11]op_constuct_array    [ 4, 0 ]
0100: [06]op_save_global       [ 37 ]
0102: [46]op_return           
0103: [06]op_save_global       [ 0 ]
0105: [45]op_func_def          [ 109 ]
0107: [37]op_jump              [ 262 ]
0109: [12]op_literal           [ 21 ]
0111: [44]op_cfunc             [ 71, 0 ]
0114: [44]op_cfunc             [ 54, 2 ]
0117: [01]op_pop_stack        
0118: [05]op_load_global       [ 29 ]
0120: [42]op_for_range_start   [ 38, 261 ]
0123: [12]op_literal           [ 22 ]
0125: [44]op_cfunc             [ 3, 0 ]
0128: [05]op_load_global       [ 31 ]
0130: [17]op_mul              
0131: [13]op_add              
0132: [06]op_save_global       [ 39 ]
0134: [12]op_literal           [ 22 ]
0136: [44]op_cfunc             [ 3, 0 ]
0139: [05]op_load_global       [ 32 ]
0141: [17]op_mul              
0142: [13]op_add              
0143: [06]op_save_global       [ 40 ]
0145: [05]op_load_global       [ 38 ]
0147: [12]op_literal           [ 13 ]
0149: [05]op_load_global       [ 27 ]
0151: [05]op_load_global       [ 39 ]
0153: [16]op_sub              
0154: [44]op_cfunc             [ 4, 2 ]
0157: [12]op_literal           [ 13 ]
0159: [05]op_load_global       [ 28 ]
0161: [05]op_load_global       [ 40 ]
0163: [16]op_sub              
0164: [44]op_cfunc             [ 4, 2 ]
0167: [05]op_load_global       [ 39 ]
0169: [05]op_load_global       [ 40 ]
0171: [11]op_constuct_array    [ 4, 0 ]
0174: [44]op_cfunc             [ 96, 1 ]
0177: [04]op_clear_temp_arrays
0178: [08]op_save_array        [ 41, 1 ]
0181: [05]op_load_global       [ 38 ]
0183: [12]op_literal           [ 13 ]
0185: [12]op_literal           [ 14 ]
0187: [44]op_cfunc             [ 4, 2 ]
0190: [12]op_literal           [ 13 ]
0192: [12]op_literal           [ 14 ]
0194: [44]op_cfunc             [ 4, 2 ]
0197: [12]op_literal           [ 13 ]
0199: [12]op_literal           [ 14 ]
0201: [44]op_cfunc             [ 4, 2 ]
0204: [12]op_literal           [ 14 ]
0206: [11]op_constuct_array    [ 4, 0 ]
0209: [44]op_cfunc             [ 104, 1 ]
0212: [04]op_clear_temp_arrays
0213: [08]op_save_array        [ 42, 1 ]
0216: [05]op_load_global       [ 38 ]
0218: [44]op_cfunc             [ 3, 0 ]
0221: [12]op_literal           [ 23 ]
0223: [17]op_mul              
0224: [12]op_literal           [ 24 ]
0226: [16]op_sub              
0227: [44]op_cfunc             [ 3, 0 ]
0230: [12]op_literal           [ 23 ]
0232: [17]op_mul              
0233: [12]op_literal           [ 24 ]
0235: [16]op_sub              
0236: [11]op_constuct_array    [ 2, 0 ]
0239: [44]op_cfunc             [ 97, 1 ]
0242: [04]op_clear_temp_arrays
0243: [08]op_save_array        [ 43, 1 ]
0246: [05]op_load_global       [ 38 ]
0248: [12]op_literal           [ 13 ]
0250: [12]op_literal           [ 25 ]
0252: [44]op_cfunc             [ 4, 2 ]
0255: [08]op_save_array        [ 44, 1 ]
0258: [43]op_for_range_jump    [ 38, 123 ]
0261: [46]op_return           
0262: [06]op_save_global       [ 45 ]
0264: [45]op_func_def          [ 268 ]
0266: [37]op_jump              [ 289 ]
0268: [12]op_literal           [ 26 ]
0270: [44]op_cfunc             [ 54, 1 ]
0273: [01]op_pop_stack        
0274: [05]op_load_global       [ 27 ]
0276: [05]op_load_global       [ 28 ]
0278: [05]op_load_global       [ 27 ]
0280: [05]op_load_global       [ 28 ]
0282: [44]op_cfunc             [ 62, 4 ]
0285: [01]op_pop_stack        
0286: [05]op_load_global       [ 45 ]
0288: [46]op_return           
0289: [06]op_save_global       [ 1 ]
0291: [45]op_func_def          [ 295 ]
0293: [37]op_jump              [ 398 ]
0295: [05]op_load_global       [ 29 ]
0297: [42]op_for_range_start   [ 38, 397 ]
0300: [05]op_load_global       [ 38 ]
0302: [07]op_load_array        [ 41, 1 ]
0305: [06]op_save_global       [ 46 ]
0307: [05]op_load_global       [ 38 ]
0309: [07]op_load_array        [ 43, 1 ]
0312: [06]op_save_global       [ 47 ]
0314: [09]op_load_field        [ 46, 2147483648 ]
0317: [09]op_load_field        [ 47, 2147483648 ]
0320: [13]op_add              
0321: [10]op_save_field        [ 46, 2147483648 ]
0324: [09]op_load_field        [ 46, 2147483648 ]
0327: [09]op_load_field        [ 46, 2147483650 ]
0330: [13]op_add              
0331: [05]op_load_global       [ 27 ]
0333: [36]op_greater_equal    
0334: [09]op_load_field        [ 46, 2147483648 ]
0337: [12]op_literal           [ 27 ]
0339: [35]op_lower_equal      
0340: [29]op_or               
0341: [39]op_jump_if_false     [ 354 ]
0343: [09]op_load_field        [ 47, 2147483648 ]
0346: [12]op_literal           [ 28 ]
0348: [17]op_mul              
0349: [10]op_save_field        [ 47, 2147483648 ]
0352: [37]op_jump              [ 354 ]
0354: [09]op_load_field        [ 46, 2147483649 ]
0357: [09]op_load_field        [ 47, 2147483649 ]
0360: [13]op_add              
0361: [10]op_save_field        [ 46, 2147483649 ]
0364: [09]op_load_field        [ 46, 2147483649 ]
0367: [09]op_load_field        [ 46, 2147483651 ]
0370: [13]op_add              
0371: [05]op_load_global       [ 28 ]
0373: [36]op_greater_equal    
0374: [09]op_load_field        [ 46, 2147483649 ]
0377: [12]op_literal           [ 27 ]
0379: [35]op_lower_equal      
0380: [29]op_or               
0381: [39]op_jump_if_false     [ 394 ]
0383: [09]op_load_field        [ 47, 2147483649 ]
0386: [12]op_literal           [ 28 ]
0388: [17]op_mul              
0389: [10]op_save_field        [ 47, 2147483649 ]
0392: [37]op_jump              [ 394 ]
0394: [43]op_for_range_jump    [ 38, 300 ]
0397: [46]op_return           
0398: [06]op_save_global       [ 2 ]
0400: [45]op_func_def          [ 404 ]
0402: [37]op_jump              [ 480 ]
0404: [05]op_load_global       [ 41 ]
0406: [05]op_load_global       [ 42 ]
0408: [05]op_load_global       [ 44 ]
0410: [44]op_cfunc             [ 70, 3 ]
0413: [01]op_pop_stack        
0414: [12]op_literal           [ 29 ]
0416: [12]op_literal           [ 20 ]
0418: [12]op_literal           [ 30 ]
0420: [44]op_cfunc             [ 69, 3 ]
0423: [01]op_pop_stack        
0424: [12]op_literal           [ 31 ]
0426: [12]op_literal           [ 20 ]
0428: [12]op_literal           [ 32 ]
0430: [05]op_load_global       [ 7 ]
0432: [05]op_load_global       [ 8 ]
0434: [44]op_cfunc             [ 22, 3 ]
0437: [12]op_literal           [ 30 ]
0439: [44]op_cfunc             [ 65, 4 ]
0442: [01]op_pop_stack        
0443: [12]op_literal           [ 29 ]
0445: [12]op_literal           [ 33 ]
0447: [11]op_constuct_array    [ 2, 1 ]
0450: [05]op_load_global       [ 27 ]
0452: [12]op_literal           [ 29 ]
0454: [16]op_sub              
0455: [12]op_literal           [ 33 ]
0457: [11]op_constuct_array    [ 2, 1 ]
0460: [05]op_load_global       [ 30 ]
0462: [44]op_cfunc             [ 68, 3 ]
0465: [04]op_clear_temp_arrays
0466: [01]op_pop_stack        
0467: [12]op_literal           [ 29 ]
0469: [12]op_literal           [ 34 ]
0471: [05]op_load_global       [ 17 ]
0473: [12]op_literal           [ 35 ]
0475: [44]op_cfunc             [ 65, 4 ]
0478: [01]op_pop_stack        
0479: [46]op_return           
0480: [06]op_save_global       [ 3 ]
0482: [45]op_func_def          [ 486 ]
0484: [37]op_jump              [ 509 ]
0486: [05]op_load_global       [ 9 ]
0488: [05]op_load_global       [ 18 ]
0490: [33]op_equal            
0491: [39]op_jump_if_false     [ 508 ]
0493: [05]op_load_global       [ 10 ]
0495: [05]op_load_global       [ 21 ]
0497: [33]op_equal            
0498: [39]op_jump_if_false     [ 506 ]
0500: [44]op_cfunc             [ 64, 0 ]
0503: [01]op_pop_stack        
0504: [37]op_jump              [ 506 ]
0506: [37]op_jump              [ 508 ]
0508: [46]op_return           
0509: [06]op_save_global       [ 4 ]
0511: [45]op_func_def          [ 515 ]
0513: [37]op_jump              [ 554 ]
0515: [05]op_load_global       [ 9 ]
0517: [05]op_load_global       [ 19 ]
0519: [33]op_equal            
0520: [39]op_jump_if_false     [ 534 ]
0522: [12]op_literal           [ 36 ]
0524: [05]op_load_global       [ 12 ]
0526: [12]op_literal           [ 37 ]
0528: [44]op_cfunc             [ 54, 3 ]
0531: [01]op_pop_stack        
0532: [37]op_jump              [ 553 ]
0534: [05]op_load_global       [ 9 ]
0536: [05]op_load_global       [ 20 ]
0538: [33]op_equal            
0539: [39]op_jump_if_false     [ 553 ]
0541: [12]op_literal           [ 38 ]
0543: [05]op_load_global       [ 13 ]
0545: [05]op_load_global       [ 14 ]
0547: [44]op_cfunc             [ 54, 3 ]
0550: [01]op_pop_stack        
0551: [37]op_jump              [ 553 ]
0553: [46]op_return           
0554: [06]op_save_global       [ 6 ]
0556: [00]op_exit             
=====================

```

## New Version using op_cfunc_void:

### Analyse


do i really need this jump's? ?! 

=>  YES. The if/else/elif need all this jumps :P
    I tried to skip them but the false jump jumps to nowhere then. 
    The false jump is wrong when i rollback the end jumps. 
    I guess this if possible but i'am a bit tired so better not now.
```
0349: [37]op_jump              [ 351 ]
0351: [09]op_load_field        [ 46, 2147483649 ]

0389: [37]op_jump              [ 391 ]
0391: [43]op_for_range_jump    [ 38, 297 ]

0495: [37]op_jump              [ 497 ]
0497: [37]op_jump              [ 499 ]
0499: [47]op_return           

0540: [37]op_jump              [ 542 ]
0542: [47]op_return    
```

the complete part where the double jump is:
```
0474: [46]op_func_def          [ 478 ] OnKey
0476: [37]op_jump              [ 500 ]
0478: [05]op_load_global       [ 9 ]
0480: [05]op_load_global       [ 18 ]
0482: [33]op_equal            
0483: [39]op_jump_if_false     [ 499 ]
0485: [05]op_load_global       [ 10 ]
0487: [05]op_load_global       [ 21 ]
0489: [33]op_equal            
0490: [39]op_jump_if_false     [ 497 ]
0492: [45]op_cfunc_void        [ 64, 0 ]
0495: [37]op_jump              [ 497 ]
0497: [37]op_jump              [ 499 ]
0499: [47]op_return      
```
- it's not the function definition this does only 476 which is fine

```
OnKey = function
    if event::type == SDL_EVENT_KEY_DOWN
       if event::key == SDLK_ESCAPE ; flux::Close; end
    end
end
```

---

```
===== TAPE DUMP =====
0000: [12]op_literal           [ 0 ]
0002: [06]op_save_global       [ 17 ]
0004: [12]op_literal           [ 1 ]
0006: [06]op_save_global       [ 18 ]
0008: [12]op_literal           [ 2 ]
0010: [06]op_save_global       [ 19 ]
0012: [12]op_literal           [ 3 ]
0014: [06]op_save_global       [ 20 ]
0016: [12]op_literal           [ 4 ]
0018: [06]op_save_global       [ 21 ]
0020: [12]op_literal           [ 5 ]
0022: [06]op_save_global       [ 22 ]
0024: [12]op_literal           [ 6 ]
0026: [06]op_save_global       [ 23 ]
0028: [12]op_literal           [ 7 ]
0030: [06]op_save_global       [ 24 ]
0032: [12]op_literal           [ 8 ]
0034: [06]op_save_global       [ 25 ]
0036: [12]op_literal           [ 9 ]
0038: [06]op_save_global       [ 26 ]
0040: [12]op_literal           [ 10 ]
0042: [06]op_save_global       [ 27 ]
0044: [12]op_literal           [ 11 ]
0046: [06]op_save_global       [ 28 ]
0048: [12]op_literal           [ 12 ]
0050: [06]op_save_global       [ 29 ]
0052: [12]op_literal           [ 13 ]
0054: [12]op_literal           [ 14 ]
0056: [03]op_dup              
0057: [11]op_constuct_array    [ 3, 0 ]
0060: [06]op_save_global       [ 30 ]
0062: [12]op_literal           [ 15 ]
0064: [06]op_save_global       [ 31 ]
0066: [12]op_literal           [ 16 ]
0068: [06]op_save_global       [ 32 ]
0070: [46]op_func_def          [ 74 ] OnSetup
0072: [37]op_jump              [ 103 ]
0074: [12]op_literal           [ 13 ]
0076: [06]op_save_global       [ 33 ]
0078: [12]op_literal           [ 17 ]
0080: [06]op_save_global       [ 34 ]
0082: [12]op_literal           [ 18 ]
0084: [06]op_save_global       [ 35 ]
0086: [12]op_literal           [ 19 ]
0088: [06]op_save_global       [ 36 ]
0090: [12]op_literal           [ 20 ]
0092: [12]op_literal           [ 15 ]
0094: [03]op_dup              
0095: [12]op_literal           [ 14 ]
0097: [11]op_constuct_array    [ 4, 0 ]
0100: [06]op_save_global       [ 37 ]
0102: [47]op_return           
0103: [06]op_save_global       [ 0 ]
0105: [46]op_func_def          [ 109 ] InitRects
0107: [37]op_jump              [ 261 ]
0109: [12]op_literal           [ 21 ]
0111: [44]op_cfunc             [ 71, 0 ]
0114: [45]op_cfunc_void        [ 54, 2 ]
0117: [05]op_load_global       [ 29 ]
0119: [42]op_for_range_start   [ 38, 260 ]
0122: [12]op_literal           [ 22 ]
0124: [44]op_cfunc             [ 3, 0 ]
0127: [05]op_load_global       [ 31 ]
0129: [17]op_mul              
0130: [13]op_add              
0131: [06]op_save_global       [ 39 ]
0133: [12]op_literal           [ 22 ]
0135: [44]op_cfunc             [ 3, 0 ]
0138: [05]op_load_global       [ 32 ]
0140: [17]op_mul              
0141: [13]op_add              
0142: [06]op_save_global       [ 40 ]
0144: [05]op_load_global       [ 38 ]
0146: [12]op_literal           [ 13 ]
0148: [05]op_load_global       [ 27 ]
0150: [05]op_load_global       [ 39 ]
0152: [16]op_sub              
0153: [44]op_cfunc             [ 4, 2 ]
0156: [12]op_literal           [ 13 ]
0158: [05]op_load_global       [ 28 ]
0160: [05]op_load_global       [ 40 ]
0162: [16]op_sub              
0163: [44]op_cfunc             [ 4, 2 ]
0166: [05]op_load_global       [ 39 ]
0168: [05]op_load_global       [ 40 ]
0170: [11]op_constuct_array    [ 4, 0 ]
0173: [44]op_cfunc             [ 96, 1 ]
0176: [04]op_clear_temp_arrays
0177: [08]op_save_array        [ 41, 1 ]
0180: [05]op_load_global       [ 38 ]
0182: [12]op_literal           [ 13 ]
0184: [12]op_literal           [ 14 ]
0186: [44]op_cfunc             [ 4, 2 ]
0189: [12]op_literal           [ 13 ]
0191: [12]op_literal           [ 14 ]
0193: [44]op_cfunc             [ 4, 2 ]
0196: [12]op_literal           [ 13 ]
0198: [12]op_literal           [ 14 ]
0200: [44]op_cfunc             [ 4, 2 ]
0203: [12]op_literal           [ 14 ]
0205: [11]op_constuct_array    [ 4, 0 ]
0208: [44]op_cfunc             [ 104, 1 ]
0211: [04]op_clear_temp_arrays
0212: [08]op_save_array        [ 42, 1 ]
0215: [05]op_load_global       [ 38 ]
0217: [44]op_cfunc             [ 3, 0 ]
0220: [12]op_literal           [ 23 ]
0222: [17]op_mul              
0223: [12]op_literal           [ 24 ]
0225: [16]op_sub              
0226: [44]op_cfunc             [ 3, 0 ]
0229: [12]op_literal           [ 23 ]
0231: [17]op_mul              
0232: [12]op_literal           [ 24 ]
0234: [16]op_sub              
0235: [11]op_constuct_array    [ 2, 0 ]
0238: [44]op_cfunc             [ 97, 1 ]
0241: [04]op_clear_temp_arrays
0242: [08]op_save_array        [ 43, 1 ]
0245: [05]op_load_global       [ 38 ]
0247: [12]op_literal           [ 13 ]
0249: [12]op_literal           [ 25 ]
0251: [44]op_cfunc             [ 4, 2 ]
0254: [08]op_save_array        [ 44, 1 ]
0257: [43]op_for_range_jump    [ 38, 122 ]
0260: [47]op_return           
0261: [06]op_save_global       [ 45 ]
0263: [46]op_func_def          [ 267 ] OnInit
0265: [37]op_jump              [ 286 ]
0267: [12]op_literal           [ 26 ]
0269: [45]op_cfunc_void        [ 54, 1 ]
0272: [05]op_load_global       [ 27 ]
0274: [05]op_load_global       [ 28 ]
0276: [05]op_load_global       [ 27 ]
0278: [05]op_load_global       [ 28 ]
0280: [45]op_cfunc_void        [ 62, 4 ]
0283: [05]op_load_global       [ 45 ]
0285: [47]op_return           
0286: [06]op_save_global       [ 1 ]
0288: [46]op_func_def          [ 292 ] OnUpdate <<<<<<<<
0290: [37]op_jump              [ 395 ]
0292: [05]op_load_global       [ 29 ] COUNT
0294: [42]op_for_range_start   [ 38, 394 ]
0297: [05]op_load_global       [ 38 ]
0299: [07]op_load_array        [ 41, 1 ]
0302: [06]op_save_global       [ 46 ]
0304: [05]op_load_global       [ 38 ]
0306: [07]op_load_array        [ 43, 1 ]
0309: [06]op_save_global       [ 47 ]
0311: [09]op_load_field        [ 46, 2147483648 ]
0314: [09]op_load_field        [ 47, 2147483648 ]
0317: [13]op_add              
0318: [10]op_save_field        [ 46, 2147483648 ]
0321: [09]op_load_field        [ 46, 2147483648 ]
0324: [09]op_load_field        [ 46, 2147483650 ]
0327: [13]op_add              
0328: [05]op_load_global       [ 27 ]
0330: [36]op_greater_equal    
0331: [09]op_load_field        [ 46, 2147483648 ]
0334: [12]op_literal           [ 27 ]
0336: [35]op_lower_equal      
0337: [29]op_or               
0338: [39]op_jump_if_false     [ 351 ]
0340: [09]op_load_field        [ 47, 2147483648 ]
0343: [12]op_literal           [ 28 ]
0345: [17]op_mul              
0346: [10]op_save_field        [ 47, 2147483648 ]
0349: [37]op_jump              [ 351 ]
0351: [09]op_load_field        [ 46, 2147483649 ]
0354: [09]op_load_field        [ 47, 2147483649 ]
0357: [13]op_add              
0358: [10]op_save_field        [ 46, 2147483649 ]
0361: [09]op_load_field        [ 46, 2147483649 ]
0364: [09]op_load_field        [ 46, 2147483651 ]
0367: [13]op_add              
0368: [05]op_load_global       [ 28 ]
0370: [36]op_greater_equal    
0371: [09]op_load_field        [ 46, 2147483649 ]
0374: [12]op_literal           [ 27 ]
0376: [35]op_lower_equal      
0377: [29]op_or               
0378: [39]op_jump_if_false     [ 391 ]
0380: [09]op_load_field        [ 47, 2147483649 ]
0383: [12]op_literal           [ 28 ]
0385: [17]op_mul              
0386: [10]op_save_field        [ 47, 2147483649 ]
0389: [37]op_jump              [ 391 ]
0391: [43]op_for_range_jump    [ 38, 297 ]
0394: [47]op_return           
0395: [06]op_save_global       [ 2 ]
0397: [46]op_func_def          [ 401 ] OnRender <<<<<<<<<
0399: [37]op_jump              [ 472 ]
0401: [05]op_load_global       [ 41 ]
0403: [05]op_load_global       [ 42 ]
0405: [05]op_load_global       [ 44 ]
0407: [45]op_cfunc_void        [ 70, 3 ]
0410: [12]op_literal           [ 29 ]
0412: [12]op_literal           [ 20 ]
0414: [12]op_literal           [ 30 ]
0416: [45]op_cfunc_void        [ 69, 3 ]
0419: [12]op_literal           [ 31 ]
0421: [12]op_literal           [ 20 ]
0423: [12]op_literal           [ 32 ]
0425: [05]op_load_global       [ 7 ]
0427: [05]op_load_global       [ 8 ]
0429: [44]op_cfunc             [ 22, 3 ]
0432: [12]op_literal           [ 30 ]
0434: [45]op_cfunc_void        [ 65, 4 ]
0437: [12]op_literal           [ 29 ]
0439: [12]op_literal           [ 33 ]
0441: [11]op_constuct_array    [ 2, 1 ]
0444: [05]op_load_global       [ 27 ]
0446: [12]op_literal           [ 29 ]
0448: [16]op_sub              
0449: [12]op_literal           [ 33 ]
0451: [11]op_constuct_array    [ 2, 1 ]
0454: [05]op_load_global       [ 30 ]
0456: [45]op_cfunc_void        [ 68, 3 ]
0459: [04]op_clear_temp_arrays
0460: [12]op_literal           [ 29 ]
0462: [12]op_literal           [ 34 ]
0464: [05]op_load_global       [ 17 ]
0466: [12]op_literal           [ 35 ]
0468: [45]op_cfunc_void        [ 65, 4 ]
0471: [47]op_return           
0472: [06]op_save_global       [ 3 ]
0474: [46]op_func_def          [ 478 ] OnKey
0476: [37]op_jump              [ 500 ]
0478: [05]op_load_global       [ 9 ]
0480: [05]op_load_global       [ 18 ]
0482: [33]op_equal            
0483: [39]op_jump_if_false     [ 499 ]
0485: [05]op_load_global       [ 10 ]
0487: [05]op_load_global       [ 21 ]
0489: [33]op_equal            
0490: [39]op_jump_if_false     [ 497 ]
0492: [45]op_cfunc_void        [ 64, 0 ]
0495: [37]op_jump              [ 497 ]
0497: [37]op_jump              [ 499 ]
0499: [47]op_return           
0500: [06]op_save_global       [ 4 ]
0502: [46]op_func_def          [ 506 ] OnMouse
0504: [37]op_jump              [ 543 ]
0506: [05]op_load_global       [ 9 ]
0508: [05]op_load_global       [ 19 ]
0510: [33]op_equal            
0511: [39]op_jump_if_false     [ 524 ]
0513: [12]op_literal           [ 36 ]
0515: [05]op_load_global       [ 12 ]
0517: [12]op_literal           [ 37 ]
0519: [45]op_cfunc_void        [ 54, 3 ]
0522: [37]op_jump              [ 542 ]
0524: [05]op_load_global       [ 9 ]
0526: [05]op_load_global       [ 20 ]
0528: [33]op_equal            
0529: [39]op_jump_if_false     [ 542 ]
0531: [12]op_literal           [ 38 ]
0533: [05]op_load_global       [ 13 ]
0535: [05]op_load_global       [ 14 ]
0537: [45]op_cfunc_void        [ 54, 3 ]
0540: [37]op_jump              [ 542 ]
0542: [47]op_return           
0543: [06]op_save_global       [ 6 ]
0545: [00]op_exit             
=====================
```

