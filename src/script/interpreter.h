// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ION_SCRIPT_INTERPRETER_H
#define ION_SCRIPT_INTERPRETER_H

#include "script/script_error.h"
#include "primitives/transaction.h"

#include <vector>
#include <stdint.h>
#include <string>

#include <boost/foreach.hpp>
#include <boost/variant.hpp>

class CTransaction;
class CScript;
class BaseSignatureChecker;

typedef std::vector<unsigned char> valtype;

bool IsDERSignature(const valtype &vchSig, bool haveHashType = true);

bool CheckSignatureEncoding(const valtype &vchSig, unsigned int flags, ScriptError* serror);

struct PrecomputedTransactionData
{
    uint256 hashPrevouts, hashSequence, hashOutputs;

    PrecomputedTransactionData(const CTransaction& tx);
};

enum SigVersion: short
{
    SIGVERSION_BASE = 0,
    SIGVERSION_WITNESS_V0 = 1,
};

uint256 SignatureHash(CScript scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType);
//uint256 SignatureHash(const CScript &scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType);

class BaseSignatureChecker
{
public:
    virtual bool CheckSig(const std::vector<unsigned char>& scriptSig, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode, SigVersion sigversion) const
    {
        return false;
    }

    virtual bool CheckLockTime(const CScriptNum& nLockTime) const
    {
         return false;
    }

    virtual bool CheckSequence(const CScriptNum& nSequence) const
    {
         return false;
    }

    virtual ~BaseSignatureChecker() {}
};

bool CheckSig(std::vector<unsigned char> vchSig, const std::vector<unsigned char> &vchPubKey, const CScript &scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType, int flags);

bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn, unsigned int flags, int nHashType);
bool VerifySignature(const CTransaction& txFrom, const CTransaction& txTo, unsigned int nIn, unsigned int flags, int nHashType);

bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, unsigned int flags, const BaseSignatureChecker& checker, ScriptError* error = NULL);

class SignatureChecker : public BaseSignatureChecker
{
private:
    const CTransaction& txTo;
    unsigned int nIn;

protected:
    virtual bool VerifySignature(const std::vector<unsigned char>& vchSig, const CPubKey& vchPubKey, const uint256& sighash) const;

public:
    SignatureChecker(const CTransaction& txToIn, unsigned int nInIn) : txTo(txToIn), nIn(nInIn) {}
    bool CheckSig(const std::vector<unsigned char>& scriptSig, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode) const;
};

class TransactionSignatureChecker : public BaseSignatureChecker
{
private:
    const CTransaction* txTo;
    unsigned int nIn;
    const CAmount amount;
    const PrecomputedTransactionData* txdata;

protected:
    virtual bool VerifySignature(const std::vector<unsigned char>& vchSig, const CPubKey& vchPubKey, const uint256& sighash) const;

public:
    TransactionSignatureChecker(const CTransaction* txToIn, unsigned int nInIn, const CAmount& amountIn) : txTo(txToIn), nIn(nInIn), amount(amountIn), txdata(NULL) {}
    TransactionSignatureChecker(const CTransaction* txToIn, unsigned int nInIn, const CAmount& amountIn, const PrecomputedTransactionData& txdataIn) : txTo(txToIn), nIn(nInIn), amount(amountIn), txdata(&txdataIn) {}
    bool CheckSig(const std::vector<unsigned char>& scriptSig, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode, SigVersion sigversion) const override;
    bool CheckLockTime(const CScriptNum& nLockTime) const override;
    bool CheckSequence(const CScriptNum& nSequence) const override;
};

class MutableTransactionSignatureChecker : public TransactionSignatureChecker
{
private:
    const CTransaction txTo;

public:
    MutableTransactionSignatureChecker(const CMutableTransaction* txToIn, unsigned int nInIn, const CAmount& amountIn) : TransactionSignatureChecker(&txTo, nInIn, amountIn), txTo(*txToIn) {}
};

bool EvalScript(std::vector<std::vector<unsigned char> >& stack, const CScript& script, unsigned int flags, const BaseSignatureChecker& checker, SigVersion sigversion, ScriptError* error = NULL);
bool EvalScript(std::vector<std::vector<unsigned char> >& stack, const CScript& script, const CTransaction& txTo, unsigned int nIn, unsigned int flags, int nHashType);


#endif // ION_SCRIPT_INTERPRETER_H