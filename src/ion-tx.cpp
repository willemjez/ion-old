// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "config/ion-config.h"
#endif

#include "core_io.h"
#include "rpcclient.h"
#include "init.h"

#include <univalue.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

static bool fCreateBlank;
static std::map<std::string,UniValue> registers;
static const int CONTINUE_EXECUTION=-1;

static int AppInitRawTx(int argc, char* argv[])
{
    //
    // Parameters
    //
    ParseParameters(argc, argv);

    // Check for -testnet or -regtest parameter (Params() calls are only valid after this clause)
    try {
        SelectParamsFromCommandLine();
    } catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return EXIT_FAILURE;
    }

    fCreateBlank = GetBoolArg("-create", false);

    if (argc<2 || mapArgs.count("-?") || mapArgs.count("-h") || mapArgs.count("-help")) 
        {
        // First part of help message is specific to this utility
        std::string strUsage = strprintf(_("%s bitcoin-tx utility version"), _(PACKAGE_NAME)) + " " + FormatFullVersion() + "\n\n" +
            _("Usage:") + "\n" +
              "  bitcoin-tx [options] <hex-tx> [commands]  " + _("Update hex-encoded bitcoin transaction") + "\n" +
              "  bitcoin-tx [options] -create [commands]   " + _("Create hex-encoded bitcoin transaction") + "\n" +
              "\n";

        fprintf(stdout, "%s", strUsage.c_str());

        strUsage = HelpMessageGroup(_("Options:"));
        strUsage += HelpMessageOpt("-?", _("This help message"));
        strUsage += HelpMessageOpt("-create", _("Create new, empty TX."));
        strUsage += HelpMessageOpt("-json", _("Select JSON output"));
        strUsage += HelpMessageOpt("-txid", _("Output only the hex-encoded transaction id of the resultant transaction."));
        strUsage += HelpMessageGroup(_("Chain selection options:"));
        strUsage += HelpMessageOpt("-testnet", _("Use the test chain"));
    
        fprintf(stdout, "%s", strUsage.c_str());

        strUsage = HelpMessageGroup(_("Commands:"));
        strUsage += HelpMessageOpt("delin=N", _("Delete input N from TX"));
        strUsage += HelpMessageOpt("delout=N", _("Delete output N from TX"));
        strUsage += HelpMessageOpt("in=TXID:VOUT(:SEQUENCE_NUMBER)", _("Add input to TX"));
        strUsage += HelpMessageOpt("locktime=N", _("Set TX lock time to N"));
        strUsage += HelpMessageOpt("nversion=N", _("Set TX version to N"));
        strUsage += HelpMessageOpt("replaceable(=N)", _("Set RBF opt-in sequence number for input N (if not provided, opt-in all available inputs)"));
        strUsage += HelpMessageOpt("outaddr=VALUE:ADDRESS", _("Add address-based output to TX"));
        strUsage += HelpMessageOpt("outpubkey=VALUE:PUBKEY[:FLAGS]", _("Add pay-to-pubkey output to TX") + ". " +
            _("Optionally add the \"W\" flag to produce a pay-to-witness-pubkey-hash output") + ". " +
            _("Optionally add the \"S\" flag to wrap the output in a pay-to-script-hash."));
        strUsage += HelpMessageOpt("outdata=[VALUE:]DATA", _("Add data-based output to TX"));
        strUsage += HelpMessageOpt("outscript=VALUE:SCRIPT[:FLAGS]", _("Add raw script output to TX") + ". " +
            _("Optionally add the \"W\" flag to produce a pay-to-witness-script-hash output") + ". " +
            _("Optionally add the \"S\" flag to wrap the output in a pay-to-script-hash."));
        strUsage += HelpMessageOpt("outmultisig=VALUE:REQUIRED:PUBKEYS:PUBKEY1:PUBKEY2:....[:FLAGS]", _("Add Pay To n-of-m Multi-sig output to TX. n = REQUIRED, m = PUBKEYS") + ". " +
            _("Optionally add the \"W\" flag to produce a pay-to-witness-script-hash output") + ". " +
            _("Optionally add the \"S\" flag to wrap the output in a pay-to-script-hash."));
        strUsage += HelpMessageOpt("sign=SIGHASH-FLAGS", _("Add zero or more signatures to transaction") + ". " +
            _("This command requires JSON registers:") +
            _("prevtxs=JSON object") + ", " +
            _("privatekeys=JSON object") + ". " +
            _("See signrawtransaction docs for format of sighash flags, JSON objects."));
        fprintf(stdout, "%s", strUsage.c_str());

        strUsage = HelpMessageGroup(_("Register Commands:"));
        strUsage += HelpMessageOpt("load=NAME:FILENAME", _("Load JSON file FILENAME into register NAME"));
        strUsage += HelpMessageOpt("set=NAME:JSON-STRING", _("Set register NAME to given JSON-STRING"));
        fprintf(stdout, "%s", strUsage.c_str());

        if (argc < 2) {
            fprintf(stderr, "Error: too few parameters\n");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    return CONTINUE_EXECUTION;
}

static void MutateTxVersion(CMutableTransaction& tx, const std::string& cmdVal)
{
    int64_t newVersion = atoi64(cmdVal);
    if (newVersion < 1 || newVersion > CTransaction::MAX_STANDARD_VERSION)
        throw std::runtime_error("Invalid TX version requested");

    tx.nVersion = (int) newVersion;
}

static void MutateTxLocktime(CMutableTransaction& tx, const std::string& cmdVal)
{
    int64_t newLocktime = atoi64(cmdVal);
    if (newLocktime < 0LL || newLocktime > 0xffffffffLL)
        throw std::runtime_error("Invalid TX locktime requested");

    tx.nLockTime = (unsigned int) newLocktime;
}

class Secp256k1Init
{
    ECCVerifyHandle globalVerifyHandle;

public:
    Secp256k1Init() {
        ECC_Start();
    }
    ~Secp256k1Init() {
        ECC_Stop();
    }
};

static void MutateTx(CMutableTransaction& tx, const std::string& command, const std::string& commandVal)
{
    std::unique_ptr<Secp256k1Init> ecc;
    

    if (command == "nversion") 
    MutateTxVersion(tx, commandVal);
    else if (command == "locktime")
    MutateTxLocktime(tx, commandVal);
/*
    else if (command == "replaceable") {
    MutateTxRBFOptIn(tx, commandVal);
    }
    
    else if (command == "delin")
    MutateTxDelInput(tx, commandVal);
    else if (command == "in")
    MutateTxAddInput(tx, commandVal);
    
    else if (command == "delout")
    MutateTxDelOutput(tx, commandVal);
    else if (command == "outaddr")
    MutateTxAddOutAddr(tx, commandVal);
    else if (command == "outpubkey") {
    if (!ecc) { ecc.reset(new Secp256k1Init()); }
    MutateTxAddOutPubKey(tx, commandVal);
    } else if (command == "outmultisig") {
    if (!ecc) { ecc.reset(new Secp256k1Init()); }
    MutateTxAddOutMultiSig(tx, commandVal);
    } else if (command == "outscript")
    MutateTxAddOutScript(tx, commandVal);
    else if (command == "outdata")
    MutateTxAddOutData(tx, commandVal);
    
    else if (command == "sign") {
    if (!ecc) { ecc.reset(new Secp256k1Init()); }
    MutateTxSign(tx, commandVal);
    }
    
    else if (command == "load")
    RegisterLoad(commandVal);
    
    else if (command == "set")
    RegisterSet(commandVal);
    
    else
        throw std::runtime_error("unknown command");
*/
}


static void OutputTxJSON(const CTransaction& tx)
{
    UniValue entry(UniValue::VOBJ);
    TxToUniv(tx, uint256(), entry);

    std::string jsonOutput = entry.write(4);
    fprintf(stdout, "%s\n", jsonOutput.c_str());
}

static void OutputTxHash(const CTransaction& tx)
{
    std::string strHexHash = tx.GetHash().GetHex(); // the hex-encoded transaction hash (aka the transaction id)

    fprintf(stdout, "%s\n", strHexHash.c_str());
}

static void OutputTxHex(const CTransaction& tx)
{
    std::string strHex = EncodeHexTx(tx);

    fprintf(stdout, "%s\n", strHex.c_str());
}

static void OutputTx(const CTransaction& tx)
{
    if (GetBoolArg("-json", false))
        OutputTxJSON(tx);
    else if (GetBoolArg("-txid", false))
        OutputTxHash(tx);
    else
        OutputTxHex(tx);
}

static std::string readStdin()
{
    char buf[4096];
    std::string ret;

    while (!feof(stdin)) {
        size_t bread = fread(buf, 1, sizeof(buf), stdin);
        ret.append(buf, bread);
        if (bread < sizeof(buf))
            break;
    }

    if (ferror(stdin))
        throw std::runtime_error("error reading stdin");

    boost::algorithm::trim_right(ret);

    return ret;
}

static int CommandLineRawTx(int argc, char* argv[])
{
    std::string strPrint;
    int nRet = 0;

    try {
        // Skip switches; Permit common stdin convention "-"
        while (argc > 1 && IsSwitchChar(argv[1][0]) &&
               (argv[1][1] != 0)) {
            argc--;
            argv++;
        }
        CMutableTransaction tx;
        int startArg;


        if (!fCreateBlank) {
            // require at least one param
            if (argc < 2)
                throw std::runtime_error("too few parameters");

            // param: hex-encoded bitcoin transaction
            std::string strHexTx(argv[1]);
            if (strHexTx == "-")                 // "-" implies standard input
                strHexTx = readStdin();

            if (!DecodeHexTx(tx, strHexTx, true))
                throw std::runtime_error("invalid transaction encoding");

            startArg = 2;
        } else
            startArg = 1;

        for (int i = startArg; i < argc; i++) {
            std::string arg = argv[i];
            std::string key, value;
            size_t eqpos = arg.find('=');
            if (eqpos == std::string::npos)
                key = arg;
            else {
                key = arg.substr(0, eqpos);
                value = arg.substr(eqpos + 1);
            }

            MutateTx(tx, key, value);
        }

        OutputTx(tx);
    }

    catch (const boost::thread_interrupted&) {
        throw;
    }
    catch (const std::exception& e) {
        strPrint = std::string("error: ") + e.what();
        nRet = EXIT_FAILURE;
    }
    catch (...) {
        PrintExceptionContinue(NULL, "CommandLineRawTx()");
        throw;
    }

    if (strPrint != "") {
        fprintf((nRet == 0 ? stdout : stderr), "%s\n", strPrint.c_str());
    }
    return nRet;
}

int main(int argc, char* argv[])
{
    try {
        int ret = AppInitRawTx(argc, argv);
        if (ret != CONTINUE_EXECUTION)
            return ret;
    }
    catch (std::exception& e) {
        PrintExceptionContinue(&e, "AppInitRawTx()");
        return EXIT_FAILURE;
    } catch (...) {
        PrintExceptionContinue(NULL, "AppInitRawTx()");
        return EXIT_FAILURE;
    }

    int ret = EXIT_FAILURE;
    try {
        ret = CommandLineRawTx(argc, argv);
    }
    catch (std::exception& e) {
        PrintExceptionContinue(&e, "CommandLineRawTx()");
    } catch (...) {
        PrintExceptionContinue(NULL, "CommandLineRawTx()");
    }
    return ret;
}