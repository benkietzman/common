#!/scripts/ocht/venv/bin/python3

import datetime
import json
import logging
import os.path
import socket
from pathlib import Path

logger = logging.getLogger('bridge')


class Bridge:
    # Global arguments across all instances
    m_port = None
    m_server = None
    m_ulModifyTime = None
    m_strApplication = None
    m_strPassword = None
    m_strUser = None
    m_strUserId = None

    def __init__(self, server, port):
        self.logger = logging.getLogger('bridge')
        self.logger.debug('creating an instance of bridge')
        self.m_server = server
        self.m_port = port

    def logger(self, strFunction, strMessage, label=None) -> bool:
        bResult = False
        if (strFunction == "log") or (strFunction == "message"):
            request = {}
            request["Section"] = "logger"
            request["Function"] = strFunction
            request["Application"] = "Bridge"
            request["Request"] = {}
            request["Request"]["Message"] = strMessage
            if type(label) is dict and label:
                request["Request"]["Label"] = label
            response = {}
            if self.request(args, request, response):
                bResult = True
        else:
            self.m_error = {"Type": "Bridge", "SubType": "logger",
                            "Message": "Please provide a valid Function:  log, message."}
        return bResult

    def nwp(self, strService, strFunction, strLoginType, nwpRequestHeader,
            nwpRequestData, nwpResponseHeader, nwpResultHeader,
            nwpResultArray) -> tuple:
        bResult = False
        request = {"Section": "nwp", "nwpService": strService,
                   "nwpFunction": strFunction, "loginType": strLoginType}
        request["Request"] = {"NwpRequestHeader": nwpRequestHeader}
        if request["Request"]["NwpRequestHeader"]["reqApp"] and self.m_strApplication:
            request["Request"]["NwpRequestHeader"]["reqApp"] = self.m_strApplication
            request["Request"]["NwpRequestData"] = nwpRequestData
            response = {}
            (stat, response) = self.request(request)
            if stat:
                if "NwpResponseHeader" in response:
                    nwpResponseHeader = response["NwpResponseHeader"]
                if "NwpResultHeader" in response:
                    nwpResultHeader = response["NwpResultHeader"]
                if "NwpResultArray" in response:
                    nwpResultArray = response["NwpResultArray"]
                if "NwpReturnStatus" in response and nwpResponseHeader["NwpReturnStatus"] == "ok":
                    bResult = True
                elif "NwpErrorMsg" in nwpResponseHeader:
                    return (False, {"Type": "Bridge", "SubType": "nwp",
                                    "Message": nwpResponseHeader["NwpErrorMsg"]})
                else:
                    return (False, {"Type": "Bridge", "SubType": "nwp",
                                    "Message": "Encountered an unknown error."})
        return (stat, response)

    def request(self, request) -> tuple:
        if self.m_strUser is None:
            return (False, {"Type": "Bridge", "SubType": "request",
                    "Message": "Please provide the User."})

        if self.m_strPassword is None:
            return {"Type": "Bridge", "SubType": "request",
                    "Message": "Please provide the Password."}

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect((self.m_server, self.m_port))
            bExit = False
            if self.m_strUser:
                request["User"] = self.m_strUser
            if self.m_strPassword:
                request["Password"] = self.m_strPassword
            if self.m_strUserID:
                request["UserID"] = self.m_strUserID
            serialized = json.dumps(request)
            serialized += "\n"
            self.logger.debug("sending: " + serialized)
            sock.sendall(serialized.encode())
            strBuff = ""
            while not bExit:
                recBytes = sock.recv(4096)
                if recBytes:
                    strBuff += str(recBytes, 'utf-8')
                    if strBuff.find("\n") != -1:
                        outargs = json.loads(strBuff[0:strBuff.find("\n")])
                        strBuff = strBuff[strBuff.find("\n") + 1:]
                        # self.logger.debug("full response: "+json.dumps(outargs))
                        if len(strBuff) <= 0:
                            bExit = True
                        if outargs.get("Status") == "okay":
                            bResult = True
                        if outargs.get("Response"):
                            return (True, outargs["Response"])
                        if outargs.get("Error"):
                            if type(outargs["Error"]) is dict:
                                self.logger.error(outargs["Error"])
                                return (False, {"Type": "Bridge", "SubType": "request",
                                                "Message": outargs["Error"]})
                else:
                    bExit = True

        return (False, {"Type": "Bridge", "SubType": "request",
                        "Message": "found multiple response lines."})

    def setCredentials(self, strApp, strUser, strPass, strUserID=None) -> bool:
        bResult = False
        if not strApp:
            return False
        if not strUser:
            return False
        if not strPass:
            return False
        self.m_strApplication = strApp
        self.m_strUser = strUser
        self.m_strPassword = strPass
        self.m_strUserID = strUserID
        return True
