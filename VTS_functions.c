/*
 * Ylib Loadrunner function library.
 * Copyright (C) 2009-2010 Raymond de Jongh <ferretproof@gmail.com> | <rdjongh@ymor.nl>
 * Copyright (C) 2010 Floris Kraak <randakar@gmail.com> | <fkraak@ymor.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _VTS_FUNC_C
#define _VTS_FUNC_C

#include "string.c"

/*****************************************************************************************************
  - VTS functies -

versie  :   0.2
auteur  :   Raymond de Jongh
datum   :   2008-11-13
            2009-09-03

benodigd:   VTS-server. Deze is normaal gesproken te vinden op een pc met loadrunner.
            Bij mij staat deze op C:\Program Files\HP\LoadRunner\bin\vtconsole.exe
            Het ip-adres en poortnummer kan je instellen via de parameters VTSServer en VTSPort
            Deze instellingen moeten overeenkomen met die van de VTSServer.


De volgende regels moeten in vuser_init() geplaatst worden:
===============================================================
//***************************
//* Load the client VTS DLL *
//***************************

lr_load_dll("vtclient.dll");
===============================================================

Deze moet geincluded worden: #include "vts2.h"
Dit kan BOVEN de vuser_init().
Dus bijvoorbeeld:
===============================================================
#include "as_web.h"
#include "vts2.h"
vuser_init()
{
    //***************************
    //* Load the client VTS DLL *
    //***************************
    lr_load_dll("vtclient.dll");
}
=============================================================== 

VTS installeren:
1) Obtain VTS2 from Mercury Support Representatives. (Downloadable Binaries)
   (je kan deze gewoon in de T:\Tools\ directory vinden. Is net handiger, toch?)
2) Unzip the file into the main LoadRunner directory (using the directory structure provided).
   (eg. "..\Mercury Interactive\LoadRunner")
   
3) Register "vtsctls.ocx" in the "..\LoadRunner\bin" directory.
  (eg. regsvr32 "c:\Program Files\Mercury Interactive\LoadRunner\bin\vtsctls.ocx")
  
VTS starten:
    C:\Program Files\HP\LoadRunner\bin\vtconsole.exe
*****************************************************************************************************/


// beeeeautiful VTS functions ;-) 
VTS_functions()
{
    lr_error_message("This function should not be called directly...\n");
    return -1;
}

// ***************************************************************************************************
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// ***************************************************************************************************

// maak een verbinding met de VTS-server
// nodig: parameter VTSServer, bevat de URL of ip-adres van de VTS-server
//        parameter VTSPort, bevat de Poortnummer van de VTS-server
int VTS_connect()
{
    // Connect to the Virtual Table Server and grab the Handle, and print it.
    PVCI ppp = vtc_connect(lr_eval_string("{VTSServer}"), atoi(lr_eval_string("{VTSPort}")), VTOPT_KEEP_ALIVE);
    int rc = vtc_get_last_error(ppp);
    
    if(rc != 0)
    {
        const char errortxt = "Can not connect to VTS: server unreachable.";
        lr_save_string(errortxt, "VTS_ERROR_MESSAGE");
        lr_error_message(errortxt);
        ppp = -1;
    }

    // lr_output_message(">> The VTS Handle is : %d", ppp);
    lr_save_int(ppp, "VTS_ppp");

    return ppp;
}

// ***************************************************************************************************
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// ***************************************************************************************************

// verbinding met VTS verbreken
int VTS_disconnect(int ppp)
{
    vtc_disconnect(ppp);
    return 0;
}



// ***************************************************************************************************
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// ***************************************************************************************************

// Voeg een waarde toe aan de onderkant van de tabel, onder voorwaarde dat deze waarde niet al bestaat!
int VTS_pushlast_unique(char * columnname, char * value)
{
    // Standard variable declarations
    PVCI           ppp;
    int            rc = 0;
    int            size;
    unsigned short status;
    int            errorcode = 0;

    ppp = VTS_connect();

    //ppp = atoi(lr_eval_string("{VTS_ppp}"));

    rc = vtc_send_if_unique(ppp, columnname, value, &status);
    //lr_log_message("result: %d .... send message status: %d", rc, status);
    if (rc != 0)
    {
        // kan niet schrijven...
        lr_save_string("Can not connect to VTS: server unreachable.","VTS_ERROR_MESSAGE");
        lr_error_message(lr_eval_string("{VTS_ERROR_MESSAGE}"));
        errorcode = -1;
    }
    else
    {
        if (status == 0)
        {
            // write failed, most likely because the value already exists in VTS
            lr_save_string("Can not write to VTS: value (most likely) already exists in VTS.","VTS_ERROR_MESSAGE");
            lr_error_message(lr_eval_string("{VTS_ERROR_MESSAGE}"));
            errorcode = -2;
        }
        else
        {   // write ok
            lr_save_string("Write to VTS: OK.","VTS_ERROR_MESSAGE");
            errorcode = 0;
        }
    }
    
    vtc_free(value);

    // Disconnect from Virtual Table Server
    VTS_disconnect(ppp);;

    return errorcode;
}





// ***************************************************************************************************
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// ***************************************************************************************************

// Voeg een waarde toe aan de onderkant van de tabel.
// klopt, het verschil tussen VTS_pushlast en VTP_pushlast_unique is 1 regeltje.
// OOIT, ik beloof het, zal ik het netter maken! Misschien...
int VTS_pushlast(char * columnname, char * value)
{
    // Standard variable declarations
    PVCI           ppp;
    int            rc = 0;
    int            size;
    unsigned short status;
    int            errorcode = 0;

    ppp = VTS_connect();

    //ppp = atoi(lr_eval_string("{VTS_ppp}"));

    rc = vtc_send_message(ppp, columnname, value, &status);
    //lr_log_message("result: %d .... send message status: %d", rc, status);
    if (rc != 0)
    {
        // kan niet schrijven...
        lr_save_string("Can not connect to VTS: server unreachable.","VTS_ERROR_MESSAGE");
        lr_error_message(lr_eval_string("{VTS_ERROR_MESSAGE}"));
        errorcode = -1;
    }
    else
    {
        if (status == 0) 
        {
            // write failed, most likely because the value already exists in VTS
            lr_save_string("Can not write to VTS: value (most likely) already exists in VTS.","VTS_ERROR_MESSAGE");
            lr_error_message(lr_eval_string("{VTS_ERROR_MESSAGE}"));
            errorcode = -2;
        }
        else
        {   // write ok
            lr_save_string("Write to VTS: OK.","VTS_ERROR_MESSAGE");
            errorcode = 0;
        }
    }
    
    vtc_free(value);

    // Disconnect from Virtual Table Server
    VTS_disconnect(ppp);;

    return errorcode;
}




// ***************************************************************************************************
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *the end* * *
// ***************************************************************************************************


// maak de hele kolom (met name "columnname") leeg
int VTS_clearColumn(char * columnname)
{
    // Standard variable declarations
    PVCI           ppp;
    int            rc = 0;
    unsigned short status;
    int            errorcode = 0;

    ppp = VTS_connect();

    if ((rc = vtc_clear_column(ppp, columnname, &status)) != 0)
    {
        lr_save_string("Can not delete column", "VTS_ERROR_MESSAGE");
        lr_error_message(lr_eval_string("{VTS_ERROR_MESSAGE}"));
        errorcode = -1;
    }
    else
    {
        lr_save_string("INFO: Content of the column is deleted", "VTS_ERROR_MESSAGE");
        lr_message(lr_eval_string("{VTS_ERROR_MESSAGE}"));
        errorcode = 0;
    }

    VTS_disconnect(ppp);

    return errorcode;
}


// ***************************************************************************************************
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *the end* * *
// ***************************************************************************************************

// lees een willekeurige cel uit de tabel columnname
int VTS_readRandom(char* columnname, char* ParameterName)
{
    // Standard variable declarations
    PVCI           ppp;
    int            rc = 0;
    unsigned short status;
    int            errorcode = 0;
    int            tablesize;
    int            rand_row;
    char           *value = NULL;

    ppp = VTS_connect();
    
    if ((rc = vtc_column_size(ppp, columnname, &tablesize)) != 0)
    {
        lr_save_string("Can not determine column size", "VTS_ERROR_MESSAGE");
        lr_error_message(lr_eval_string("{VTS_ERROR_MESSAGE}"));
        errorcode = -1;
    }

    //lr_log_message("tablesize: %d\n", tablesize);
    
    rand_row = rand() % tablesize + 1;

    if ((rc = vtc_query_column(ppp, columnname, rand_row, &value)) != 0)
    {
        lr_error_message("******************** VTS Error - Query Return Code = %d", rc);
    }
    else
    {
        //lr_output_message("******************** Query Column 1 Result = %s", value);
        lr_save_string(value,ParameterName);
    }

    
    // Maak het geheugen weer vrij
    vtc_free(value);
    
    //    Disconnect from Virtual Table Server
    VTS_disconnect(ppp);
}





// lees een willekeurige kolom uit de VTS-database.
// De naam van de colums wordt in parameter {databasevelden} bepaald.
int VTS_readRandomMultipleColumns(char* columnname)
{
    // Standard variable declarations
    PVCI           ppp;
    int            rc = 0;
    unsigned short status;
    int            errorcode = 0;
    int            tablesize;
    int            rand_row;
    char           *value = NULL;

    ppp = VTS_connect();

    tablesize=0;
    lr_save_string(lr_eval_string("{databasevelden}"), "databaseveld");
    do
    {
        if (tablesize==0)
        {
            if ((rc = vtc_column_size(ppp, lr_eval_string("{databaseveld}"), &tablesize)) != 0)
            {
                lr_save_string("Can not determine column size", "VTS_ERROR_MESSAGE");
                lr_error_message(lr_eval_string("{VTS_ERROR_MESSAGE}"));
                errorcode = -1;
            }
            rand_row = rand() % tablesize + 1;
        }
        lr_message("Tablesize: %d", tablesize);
        lr_message("rand_row: %d", rand_row);

        if ((rc = vtc_query_column(ppp, lr_eval_string("{databaseveld}"), rand_row, &value)) != 0)
        {
            lr_error_message("******************** VTS Error - Query Return Code = %d", rc);
        }
        else
        {
            // lr_output_message("******************** Query Column 1 Result = %s", value);
            lr_save_string(value,lr_eval_string("{databaseveld}"));
        }
        lr_save_string(lr_eval_string("{databasevelden}"), "databaseveld");
    }
    while (strcmp(lr_eval_string("{databaseveld}"), "einde") != 0);
    
    // Maak het geheugen weer vrij
    vtc_free(value);
    
    //    Disconnect from Virtual Table Server
    VTS_disconnect(ppp);
}


// ***************************************************************************************************
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// ***************************************************************************************************
// input: een waarde waarin de kolomnamen staan die uitgelezen moeten worden 
// output: parameter die de naam heeft van de kolomnamen. 
// verwerking: de waarde van de opgegeven kolom worden in een parameter gezet en vervolgens wordt deze waarden 
//             uit de database verwijderd. Zodoende kan een deze waarde nooit 2x gebruikt worden.
int VTS_popfirst(char *columnname)
{
    //    Standard variable declarations
    PVCI           ppp;
    int            rc = 0;
    int            size;
    unsigned short status;
    int            errorcode = 0;
    
    ppp = lrvtc_connect(lr_eval_string("{VTSServer}"), atoi(lr_eval_string("{VTSPort}")), 0);
    if( (rc = lrvtc_retrieve_message(columnname)) != 0)
    {
        lr_error_message("******************** VTS Error - Query Return Code = %d", rc);
    }
    else
    {
        // dit werkt niet helemaal goed. Dit geeft de tekst "columname" terug, ipv de inhoud van {columnname}
        lr_output_message ("Retrieved value is : %s", y_get_parameter(columnname));
    }
    lrvtc_disconnect();

    return errorcode;
}


// ***************************************************************************************************
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// ***************************************************************************************************
// VTS_push_multiple_columns()
//
//     Voeg data toe aan meerdere kolommen tegelijk, aan de onderkant van de database.
//     De kolomnamen staan in 1 string, gescheiden door een punt-komma.
//     De data staan in 1 string, gescheiden door een punt-komma.
// 
// Voorbeeld:
//     VTS_push_multiple_columns("VOORNAAM,ACHTERNAAM,ADRES", "Pietje;Puk;Wegiswegweg 3");
int VTS_push_multiple_columns_unique(char *columnnames, char *data)
{
    //    Standard variable declarations
    PVCI           ppp;
    int            rc = 0;
    unsigned short status;
    int            errorcode = 0;

    ppp = lrvtc_connect(lr_eval_string("{VTSServer}"), atoi(lr_eval_string("{VTSPort}")), 0);

    rc=lrvtc_send_row1(columnnames, data, ";", VTSEND_STACKED_UNIQUE);
    if(rc != 0)
    {
        lr_save_string("Can not write to columns: ", "VTS_ERROR_MESSAGE");
        lr_error_message(lr_eval_string("{VTS_ERROR_MESSAGE}"));
        errorcode = -1;
    }

    lrvtc_disconnect();
    return errorcode;
}


// ***************************************************************************************************
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// ***************************************************************************************************
// VTS_popfirstMultipleColumns()
// 
// Haal uit VTS de bovenste rij uit de opgegeven kolommen.
// deze kolommen staan in 1 string, gescheiden door een punt-komma.
// 
// Voorbeeld:
//     VTS_popfirstMultipleColumns("Voornaam;Achternaam;Adres");
//        Het resultaat wordt dan in {Voornaam}, {Acternaam} en {Adres} geplaatst.
// 
//    snelheid:
//     wanneer in VTS 1 miljoen records staan, en 1000x het volgende wordt uitgevoerd:
//           VTS_push_multiple_columns("CRDNUM;UTN;EMBNM1", "123123123;123123123;JANSEN");
//            VTS_popfirstMultipleColumns2("CRDNUM;UTN;EMBNM1");
//     dan duurt dat ca. 12,5 sec. Dat is gemiddeld dus per push en pop: 12,5 msec. 
int VTS_popfirstMultipleColumns(char *gewenste_databasevelden)
{
    // Standard variable declarations
    PVCI           ppp;
    int            rc = 0;
    unsigned short status;
    int            errorcode = 0;

    ppp = lrvtc_connect(lr_eval_string("{VTSServer}"), atoi(lr_eval_string("{VTSPort}")), 0);

    if( (rc = lrvtc_retrieve_messages1(gewenste_databasevelden, ";")) != 0)
    {
        lr_error_message("******************** VTS Error - Query Return Code = %d", rc);
    }
    else
    {
        // lr_output_message("******************** Query Column 1 Result = %s", value);
        // lr_save_string(value,lr_eval_string("{databaseveld}"));
    }
    lrvtc_disconnect();

    return errorcode;
}



// ***************************************************************************************************
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// ***************************************************************************************************
// VTS_push_multiple_columns()
//
//     Voeg data toe aan meerdere kolommen tegelijk, aan de onderkant van de database.
//     De kolomnamen staan in 1 string, gescheiden door een punt-komma.
//     De data staan in 1 string, gescheiden door een punt-komma.
// 
// Voorbeeld:
//     VTS_push_multiple_columns("VOORNAAM,ACHTERNAAM,ADRES", "Pietje;Puk;Wegiswegweg 3");
int VTS_push_multiple_columns(char* columnnames, char* data)
{
    //    Standard variable declarations
    PVCI           ppp;
    int            rc = 0;
    unsigned short status;
    int            errorcode = 0;

    ppp = lrvtc_connect(lr_eval_string("{VTSServer}"), atoi(lr_eval_string("{VTSPort}")), 0);

    rc=lrvtc_send_row1(columnnames, data, ";", VTSEND_SAME_ROW);
    if (rc != 0)
    {
        lr_save_string("Can not write to columns: ", "VTS_ERROR_MESSAGE");
        lr_error_message(lr_eval_string("{VTS_ERROR_MESSAGE}"));
        errorcode = -1;
    }

    lrvtc_disconnect();
    return errorcode;
}

// --------------------------------------------------------------------------------------------------
#endif // _VTS_FUNC_C