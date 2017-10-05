// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "config/ion-config.h"
#endif

//#include "script/standard.h"        //OK
//#include "script/script_error.h"    //OK
//#include "script/sigcache.h"        //OK
//#include "script/script.h"          //OK
#include "script/interpreter.h"     //OK
//#include "script/ismine.h"          //OK
//#include "script/sign.h"            //OK
//#include "serialize.h"              //OK
//#include "uint256.h"                //OK
//#include "amount.h"                 //OK
//#include "compressor.h"             //OK
//#include "txmempool.h"              //OK
//#include "main.h"                   //OK
//#include "pubkey.h"                 //OK
//#include "chainparams.h"            //OK
//#include "primitives/transaction.h" //OK
//#include "policy/policy.h"          //OK
//#include "base58.h"                 //OK
//#include "keystore.h"               //OK
//#include "coins.h"                  //OK
//#include "utilstrencodings.h"       //OK

//#include "wallet.h"                 //OK
//#include "proofs.h"                 //OK


/*
CTxDestination is not a class; ismine.h, script/script.h, script/isstandard.h need to forward declare or include it.
*/


int main(int argc, char* argv[])
{
    return 0;
}