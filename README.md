# ZoekWG_CLI

ZoekWG_CLI is een Windows programma dat werkt via 'command line interface', d.i. de opdrachtprompt van Windows.
Het aanvaard als parameter een xlsx-adressenbestand, dat het zal aanvullen met overeenkomstige namen en telefoonnummers.

Hoe te beginnen?
Open het programma (op klikken). Tekst en uitleg verschijnen in een opdrachtpromptvenster.

Hoe is het gemaakt?
Het is volledig in C geschreven, maakt gebruik van Windows API's en werd gecompileerd met PellesC.
Voor het programma te maken werden volgende onderdelen gebruikt (voor de licenties hiervan, open het programma):
* xlsxio (dat gebruikt maakt van libzip, dat gebruik maakt van zlib, en expat)
* xlsxwriter (dat gebruikt maakt van minizip, dat gebruik maakt van zlib)
