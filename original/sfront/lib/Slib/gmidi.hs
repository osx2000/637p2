
//////////////////////////////////////////////////////////////////////////
//
//   Slib, Sfront's SAOL library
//   This file: The gmidi library (General MIDI definitions)
//
// Copyright (c) 1999-2006, Regents of the University of California
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//
//  Neither the name of the University of California, Berkeley nor the
//  names of its contributors may be used to endorse or promote products
//  derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//    Maintainer: John Lazzaro, lazzaro@cs.berkeley.edu
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//                                                
//
// This library contains the definitions for the 128 General MIDI preset
// numbers, as well as the MIDI Note Numbers for the General MIDI drum
// sounds. 
//
// The General MIDI Instrument Names:
//
// 
//        Defined name           Preset   Description
//
//        GM_ACOUSTICGRAND       0        Acoustic Grand           
//        GM_BRIGHTACOUSTIC      1        Bright Acoustic           
//        GM_ELECTRICGRAND       2        Electric Grand            
//        GM_HONKYTONK           3        Honky-Tonk                
//        GM_ELECTRICPIANO1      4        Electric Piano 1          
//        GM_ELECTRICPIANO2      5        Electric Piano 2          
//        GM_HARPSICHORD         6        Harpsichord               
//        GM_CLAVINET            7        Clavinet                  
//        GM_CELESTA             8        Celesta
//        GM_GLOCKENSPIEL        9        Glockenspiel
//        GM_MUSICBOX            10       Music Box
//        GM_VIBRAPHONE          11       Vibraphone
//        GM_MARIMBA             12       Marimba
//        GM_XYLOPHONE           13       Xylophone
//        GM_TUBULARBELLS        14       Tubular Bells
//        GM_DULCIMER            15       Dulcimer
//        GM_DRAWBARORGAN        16       Drawbar Organ             
//        GM_PERCUSSIVEORGAN     17       Percussive Organ          
//        GM_ROCKORGAN           18       Rock Organ                
//        GM_CHURCHORGAN         19       Church Organ              
//        GM_REEDORGAN           20       Reed Organ                
//        GM_ACCORDIAN           21       Accoridan                 
//        GM_HARMONICA           22       Harmonica                 
//        GM_TANGOACCORDIAN      23       Tango Accordian           
//        GM_NYLONSTRINGGUITAR   24       Nylon String Guitar
//        GM_STEELSTRINGUITAR    25       Steel String Guitar
//        GM_ELECTRICJAZZGUITAR  26       Electric Jazz Guitar
//        GM_ELECTRICCLEANGUITAR 27       Electric Clean Guitar
//        GM_ELECTRICMUTEDGUITAR 28       Electric Muted Guitar
//        GM_OVERDRIVERGUITAR    29       Overdriven Guitar
//        GM_DISTORTIONGUITAR    30       Distortion Guitar
//        GM_GUITARHARMONICS     31       Guitar Harmonics
//        GM_ACOUSTICBASS        32       Acoustic Bass             
//        GM_ELECTRICBASSFINGER  33       Electric Bass(finger)     
//        GM_ELECTRICBASSPICK    34       Electric Bass(pick)       
//        GM_FRETLESSBASS        35       Fretless Bass             
//        GM_SLAPBASS1           36       Slap Bass 1               
//        GM_SLAPBASS2           37       Slap Bass 2               
//        GM_SYNTHBASS1          38       Synth Bass 1              
//        GM_SYNTHBASS2          39       Synth Bass 2              
//        GM_VIOLIN              40       Violin
//        GM_VIOLA               41       Viola
//        GM_CELLO               42       Cello
//        GM_CONTRABASS          43       Contrabass
//        GM_TREMELOSTRINGS      44       Tremolo Strings
//        GM_PIZZICATOSTRINGS    45       Pizzicato Strings
//        GM_OCHESTRALSTRINGS    46       Orchestral Strings
//        GM_TIMPANI             47       Timpani
//        GM_STRINGENSEMBLE1     48       String Ensemble 1         
//        GM_STRINGENSEMBLE2     49       String Ensemble 2         
//        GM_SYNTHSTRINGS1       50       SynthStrings 1            
//        GM_SYNTHSTRINGS2       51       SynthStrings 2            
//        GM_CHIORAAHS           52       Choir Aahs                
//        GM_VOICEOOHS           53       Voice Oohs                
//        GM_SYNTHVOICE          54       Synth Voice               
//        GM_ORCHESTRAHIT        55       Orchestra Hit             
//        GM_TRUMPET             56       Trumpet
//        GM_TROMBONE            57       Trombone
//        GM_TUBA                58       Tuba
//        GM_MUTEDTRUMPET        59       Muted Trumpet
//        GM_FRENCHHORN          60       French Horn
//        GM_BRASSSECTION        61       Brass Section
//        GM_SYNTHBRASS1         62       SynthBrass 1
//        GM_SYNTHBRASS2         63       SynthBrass 2
//        GM_SOPRANOSAX          64       Soprano Sax               
//        GM_ALTOSAX             65       Alto Sax      
//        GM_TENORSAX            66       Tenor Sax     
//        GM_BARITONESAX         67       Baritone Sax  
//        GM_OBOE                68       Oboe          
//        GM_ENGLISHHORN         69       English Horn  
//        GM_BASSOON             70       Bassoon       
//        GM_CLARINET            71       Clarinet      
//        GM_PICCOLO             72       Piccolo
//        GM_FLUTE               73       Flute
//        GM_RECORDER            74       Recorder
//        GM_PANFLUTE            75       Pan Flute
//        GM_BLOWNBOTTLE         76       Blown Bottle
//        GM_SHAKUHACHI          77       Skakuhachi
//        GM_WHISTLE             78       Whistle
//        GM_OCARINA             79       Ocarina
//        GM_LEAD1SQUARE         80       Lead 1 (square)         
//        GM_LEAD2SAWTOOTH       81       Lead 2 (sawtooth)       
//        GM_LEAD3CALLIOPE       82       Lead 3 (calliope)       
//        GM_LEAD4CHIFF          83       Lead 4 (chiff)          
//        GM_LEAD5CHARANG        84       Lead 5 (charang)        
//        GM_LEAD6VOICE          85       Lead 6 (voice)          
//        GM_LEAD7FIFTHS         86       Lead 7 (fifths)         
//        GM_LEAD8BASSPLUSLEAD   87       Lead 8 (bass+lead)      
//        GM_PAD1NEWAGE          88       Pad 1 (new age)
//        GM_PAD2WARM            89       Pad 2 (warm)
//        GM_PAD3POLYSYNTH       90       Pad 3 (polysynth)
//        GM_PAD4CHOIR           91       Pad 4 (choir)
//        GM_PAD5BOWED           92       Pad 5 (bowed)
//        GM_PAD6METALLIC        93       Pad 6 (metallic)
//        GM_PAD7HALO            94       Pad 7 (halo)
//        GM_PAD8SWEEP           95       Pad 8 (sweep)
//        GM_FX1RAIN             96       FX 1 (rain)           
//        GM_FX2SOUNDTRACK       97       FX 2 (soundtrack)     
//        GM_FX3CRYSTAL          98       FX 3 (crystal)        
//        GM_FX4ATMOSPHERE       99       FX 4 (atmosphere)    
//        GM_FX5BRIGHTNESS      100       FX 5 (brightness)    
//        GM_FX6GLOBLINS        101       FX 6 (goblins)       
//        GM_FX7ECHOES          102       FX 7 (echoes)        
//        GM_FX8SCIFI           103       FX 8 (sci-fi)        
//        GM_SITAR              104       Sitar
//        GM_BANJO              105       Banjo
//        GM_SHAMISEN           106       Shamisen
//        GM_KOTO               107       Koto
//        GM_KALIMBA            108       Kalimba
//        GM_BAGPIPE            109       Bagpipe
//        GM_FIDDLE             110       Fiddle
//        GM_SHANAI             111       Shanai
//        GM_TINKEBELL          112       Tinkle Bell        
//        GM_AGOGO              113       Agogo              
//        GM_STEELDRUMS         114       Steel Drums        
//        GM_WOODBLOCK          115       Woodblock          
//        GM_TAIKODRUM          116       Taiko Drum         
//        GM_MELODICTOM         117       Melodic Tom        
//        GM_SYNTHDRUM          118       Synth Drum         
//        GM_REVERSECYMBAL      119       Reverse Cymbal     
//        GM_GUITARFRETNOISE    120       Guitar Fret Noise
//        GM_BREATHNOISE        121       Breath Noise
//        GM_SEASHORE           122       Seashore
//        GM_BIRDTWEET          123       Bird Tweet
//        GM_TELEPHONERING      124       Telephone Ring
//        GM_HELICOPTER         125       Helicopter
//        GM_APPLAUSE           126       Applause
//        GM_GUNSHOT            127       Gunshot
//
//
//  The General MIDI Drum Note Numbers
//
//        Definition             Note     Description
// 
//        GMD_ACOUSTICBASSDRUM   35       Acoustic Bass Drum     
//        GMD_BASSDRUM1          36       Bass Drum 1            
//        GMD_SIDESTICK          37       Side Stick             
//        GMD_ACOUSTICSNARE      38       Acoustic Snare         
//        GMD_HANDCLAP           39       Hand Clap              
//        GMD_ELECTRICSNARE      40       Electric Snare         
//        GMD_LOWFLOORTOM        41       Low Floor Tom          
//        GMD_CLOSEDHIHAT        42       Closed Hi-Hat          
//        GMD_HIGHFLOORTOM       43       High Floor Tom         
//        GMD_PEDALHIHAT         44       Pedal Hi-Hat           
//        GMD_LOWTOM             45       Low Tom
//        GMD_OPENHIHAT          46       Open Hi-Hat            
//        GMD_LOWMIDTOM          47       Low-Mid Tom            
//        GMD_HIMIDTOM           48       Hi-Mid Tom             
//        GMD_CRASHCYMBAL1       49       Crash Cymbal 1         
//        GMD_HITOM              50       High Tom               
//        GMD_RIDECYMBAL1        51       Ride Cymbal 1          
//        GMD_CHINESECYMBAL      52       Chinese Cymbal         
//        GMD_RIDEBELL           53       Ride Bell              
//        GMD_TAMBOURINE         54       Tambourine             
//        GMD_SPLASHCYMBAL       55       Splash Cymbal          
//        GMD_COWBELL            56       Cowbell                
//        GMD_CRASHCYMBAL2       57       Crash Cymbal 2         
//        GMD_VIBRASLAP          58       Vibraslap
//        GMD_RIDECYMBAL2        59       Ride Cymbal 2
//        GMD_HIBONGO            60       Hi Bongo
//        GMD_LOBONGO            61       Low Bongo
//        GMD_MUTEHICONGA        62       Mute Hi Conga
//        GMD_OPENHICONGA        63       Open Hi Conga
//        GMD_LOWCONGA           64       Low Conga
//        GMD_HIGHTIMBALE        65       High Timbale
//        GMD_LOWTIMBALE         66       Low Timbale
//        GMD_HIGHAGOGO          67       High Agogo
//        GMD_LOWAGOGO           68       Low Agogo
//        GMD_CABASA             69       Cabasa
//        GMD_MARACAS            70       Maracas
//        GMD_SHORTWHISTLE       71       Short Whistle
//        GMD_LONGWHISTLE        72       Long Whistle
//        GMD_SHORTGUIRO         73       Short Guiro
//        GMD_LONGGUIRO          74       Long Guiro
//        GMD_CLAVES             75       Claves
//        GMD_HIWOODBLOCK        76       Hi Wood Block
//        GMD_LOWWOODBLOCK       77       Low Wood Block
//        GMD_MUTECUICA          78       Mute Cuica
//        GMD_OPENCUICA          79       Open Cuica
//        GMD_MUTETRIANGLE       80       Mute Triangle
//        GMD_OPENTRIANGLE       81      Open Triangle
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//                                                
// library definitions begin here
//

#ifndef SLIB_GMIDI
#define SLIB_GMIDI

#define GM_ACOUSTICGRAND       0    
#define GM_BRIGHTACOUSTIC      1    
#define GM_ELECTRICGRAND       2    
#define GM_HONKYTONK           3    
#define GM_ELECTRICPIANO1      4    
#define GM_ELECTRICPIANO2      5    
#define GM_HARPSICHORD         6    
#define GM_CLAVINET            7    
#define GM_CELESTA             8    
#define GM_GLOCKENSPIEL        9    
#define GM_MUSICBOX            10   
#define GM_VIBRAPHONE          11   
#define GM_MARIMBA             12   
#define GM_XYLOPHONE           13   
#define GM_TUBULARBELLS        14   
#define GM_DULCIMER            15   
#define GM_DRAWBARORGAN        16   
#define GM_PERCUSSIVEORGAN     17   
#define GM_ROCKORGAN           18   
#define GM_CHURCHORGAN         19   
#define GM_REEDORGAN           20   
#define GM_ACCORDIAN           21   
#define GM_HARMONICA           22   
#define GM_TANGOACCORDIAN      23   
#define GM_NYLONSTRINGGUITAR   24   
#define GM_STEELSTRINGUITAR    25   
#define GM_ELECTRICJAZZGUITAR  26   
#define GM_ELECTRICCLEANGUITAR 27   
#define GM_ELECTRICMUTEDGUITAR 28   
#define GM_OVERDRIVERGUITAR    29   
#define GM_DISTORTIONGUITAR    30   
#define GM_GUITARHARMONICS     31   
#define GM_ACOUSTICBASS        32   
#define GM_ELECTRICBASSFINGER  33   
#define GM_ELECTRICBASSPICK    34   
#define GM_FRETLESSBASS        35   
#define GM_SLAPBASS1           36   
#define GM_SLAPBASS2           37   
#define GM_SYNTHBASS1          38   
#define GM_SYNTHBASS2          39   
#define GM_VIOLIN              40   
#define GM_VIOLA               41   
#define GM_CELLO               42   
#define GM_CONTRABASS          43   
#define GM_TREMELOSTRINGS      44   
#define GM_PIZZICATOSTRINGS    45   
#define GM_OCHESTRALSTRINGS    46   
#define GM_TIMPANI             47   
#define GM_STRINGENSEMBLE1     48   
#define GM_STRINGENSEMBLE2     49   
#define GM_SYNTHSTRINGS1       50   
#define GM_SYNTHSTRINGS2       51   
#define GM_CHIORAAHS           52   
#define GM_VOICEOOHS           53   
#define GM_SYNTHVOICE          54   
#define GM_ORCHESTRAHIT        55   
#define GM_TRUMPET             56   
#define GM_TROMBONE            57   
#define GM_TUBA                58   
#define GM_MUTEDTRUMPET        59   
#define GM_FRENCHHORN          60   
#define GM_BRASSSECTION        61   
#define GM_SYNTHBRASS1         62   
#define GM_SYNTHBRASS2         63   
#define GM_SOPRANOSAX          64   
#define GM_ALTOSAX             65   
#define GM_TENORSAX            66   
#define GM_BARITONESAX         67   
#define GM_OBOE                68   
#define GM_ENGLISHHORN         69   
#define GM_BASSOON             70   
#define GM_CLARINET            71   
#define GM_PICCOLO             72   
#define GM_FLUTE               73   
#define GM_RECORDER            74   
#define GM_PANFLUTE            75   
#define GM_BLOWNBOTTLE         76   
#define GM_SHAKUHACHI          77   
#define GM_WHISTLE             78   
#define GM_OCARINA             79   
#define GM_LEAD1SQUARE         80   
#define GM_LEAD2SAWTOOTH       81   
#define GM_LEAD3CALLIOPE       82   
#define GM_LEAD4CHIFF          83   
#define GM_LEAD5CHARANG        84   
#define GM_LEAD6VOICE          85   
#define GM_LEAD7FIFTHS         86   
#define GM_LEAD8BASSPLUSLEAD   87   
#define GM_PAD1NEWAGE          88   
#define GM_PAD2WARM            89   
#define GM_PAD3POLYSYNTH       90   
#define GM_PAD4CHOIR           91   
#define GM_PAD5BOWED           92   
#define GM_PAD6METALLIC        93   
#define GM_PAD7HALO            94   
#define GM_PAD8SWEEP           95   
#define GM_FX1RAIN             96  
#define GM_FX2SOUNDTRACK       97  
#define GM_FX3CRYSTAL          98  
#define GM_FX4ATMOSPHERE       99  
#define GM_FX5BRIGHTNESS      100  
#define GM_FX6GLOBLINS        101  
#define GM_FX7ECHOES          102  
#define GM_FX8SCIFI           103  
#define GM_SITAR              104  
#define GM_BANJO              105  
#define GM_SHAMISEN           106  
#define GM_KOTO               107  
#define GM_KALIMBA            108  
#define GM_BAGPIPE            109  
#define GM_FIFFLE             110  
#define GM_SHANAI             111  
#define GM_TINKEBELL          112  
#define GM_AGOGO              113  
#define GM_STEELDRUMS         114  
#define GM_WOODBLOCK          115  
#define GM_TAIKODRUM          116  
#define GM_MELODICTOM         117  
#define GM_SYNTHDRUM          118  
#define GM_REVERSECYMBAL      119  
#define GM_GUITARFRETNOISE    120  
#define GM_BREATHNOISE        121  
#define GM_SEASHORE           122  
#define GM_BIRDTWEET          123  
#define GM_TELEPHONERING      124  
#define GM_HELICOPTER         125  
#define GM_APPLAUSE           126  
#define GM_GUNSHOT            127  

#define GMD_ACOUSTICBASSDRUM   35   
#define GMD_BASSDRUM1          36   
#define GMD_SIDESTICK          37   
#define GMD_ACOUSTICSNARE      38   
#define GMD_HANDCLAP           39   
#define GMD_ELECTRICSNARE      40   
#define GMD_LOWFLOORTOM        41   
#define GMD_CLOSEDHIHAT        42   
#define GMD_HIGHFLOORTOM       43   
#define GMD_PEDALHIHAT         44   
#define GMD_LOWTOM             45
#define GMD_OPENHIHAT          46   
#define GMD_LOWMIDTOM          47   
#define GMD_HIMIDTOM           48   
#define GMD_CRASHCYMBAL1       49   
#define GMD_HITOM              50   
#define GMD_RIDECYMBAL1        51   
#define GMD_CHINESECYMBAL      52   
#define GMD_RIDEBELL           53   
#define GMD_TAMBOURINE         54   
#define GMD_SPLASHCYMBAL       55   
#define GMD_COWBELL            56   
#define GMD_CRASHCYMBAL2       57   
#define GMD_VIBRASLAP          58   
#define GMD_RIDECYMBAL2        59   
#define GMD_HIBONGO            60   
#define GMD_LOBONGO            61   
#define GMD_MUTEHICONGA        62   
#define GMD_OPENHICONGA        63   
#define GMD_LOWCONGA           64   
#define GMD_HIGHTIMBALE        65   
#define GMD_LOWTIMBALE         66   
#define GMD_HIGHAGOGO          67   
#define GMD_LOWAGOGO           68   
#define GMD_CABASA             69   
#define GMD_MARACAS            70   
#define GMD_SHORTWHISTLE       71   
#define GMD_LONGWHISTLE        72   
#define GMD_SHORTGUIRO         73   
#define GMD_LONGGUIRO          74   
#define GMD_CLAVES             75   
#define GMD_HIWOODBLOCK        76   
#define GMD_LOWWOODBLOCK       77   
#define GMD_MUTECUICA          78   
#define GMD_OPENCUICA          79   
#define GMD_MUTETRIANGLE       80   
#define GMD_OPENTRIANGLE       81   


#endif // SLIB_GMIDI 

