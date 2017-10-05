// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ION_SCRIPT_STANDARDKEYSTORE_H
#define ION_SCRIPT_STANDARDKEYSTORE_H

#include "script/standard.h"
#include "keystore.h"

//#include <boost/variant.hpp>

//#include <stdint.h>

class CKeyStore;
class CScript;

bool Solver(const CKeyStore& keystore, const CScript& scriptPubKey, uint256 hash, int nHashType, CScript& scriptSigRet, txnouttype& whichTypeRet);
void ExtractAffectedKeys(const CKeyStore &keystore, const CScript& scriptPubKey, std::vector<CKeyID> &vKeys);

#endif // ION_SCRIPT_STANDARDKEYSTORE_H
