import socket
import os


class Warden:
    # Global arguments across all instances
    m_application = None
    m_unixsock = None
    m_error = None

    def __init__(self, application, unixsock):
        self.m_application = application
        self.m_unixsock = unixsock

    # def __del__(self):
    #    self.m_strConf = None

    def authn(self, data=dict()) -> tuple:
        return self.request("authn", data)

    def authz(self, data=dict()) -> tuple:
        return self.request("authz", data)

    def bridge(self, data: dict) -> tuple:
        return self.request("bridge", data)

    def bridge_authn(self, user: str, pwd: str) -> tuple:
        return self.bridge_authz(user, pwd)

    def bridge_authz(self, user: str, pwd: str, data=dict()) -> tuple:
        data["User"] = user
        data["Password"] = pwd
        return self.bridge(data)

    def central(self, data: dict) -> tuple:
        return self.request("central", data)

    def central_user(self, user: str, data: dict) -> tuple:
        data["User"] = user
        return self.central(data)

    def password(self, data: dict) -> tuple:
        return self.request("password", data)


    def password_application(self, app: str, user: str, pwd: str, stype='') -> tuple:
        request = {"Application": app, "User": user, "Password": pwd}
        if stype:
            request["Type"] = stype
        return self.password(request)


    def password_user(self, user: str, pwd: str) -> tuple:
        request = {"User": user, "Password": pwd}
        return self.password(request)


    def request(self, module: str, request: dict) -> tuple:
        request['Module'] = module;
        (status, response) = self.send(request)
        if status:
            if response and response["Data"]:
                return (status, response["Data"])
            else:
                return (status, dict())
        else:
            return (status, response)


    def send(self, request: dict) -> tuple:
        if os.path.exists(self.m_unixsock):
            with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
                sock.connect(self.m_unixsock)
                bExit = False
                serialized = json.dumps(request)
                serialized += "\n"
                self.logger.debug("sending: "+serialized)
                sock.sendall(serialized.encode())
                strBuff = ""
                while not bExit:
                    recBytes = sock.recv(4096)
                    if recBytes:
                        strBuff += str(recBytes, 'utf-8')
                        if strBuff.find("\n") != -1:
                            outargs = json.loads(strBuff[0:strBuff.find("\n")])
                            strBuff = strBuff[strBuff.find("\n")+1:]
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

            return (False, {"Type": "Warden", "SubType": "request",
                            "Message": "found multiple response lines."})
        else:
            return (False, {"Type": "Warden", "SubType": "request",
                            "Message": "Could not connect Warden socket."})


    def set_application(self, application: str) -> None:
        self.m_application = application

    def vault(self, function: str, keys=list(), data=dict()) -> tuple:
        if self.m_application:
            request = dict()
            request["Function"] = function
            keys.insert(0, self.m_application)
            request["Keys"] = keys
            if data:
                request["Data"] = data
            (status, resp) = self.request("vault", request)
            return (status, resp)
        else:
            return (False, {"Type": "Warden", "SubType": "request",
                            "Message": "Please provide the Application."})

    def vault_add(self, keys=list(), data=dict()) -> tuple:
        return self.vault("add", keys, data)

    def vault_remove(self, keys=list()) -> tuple:
        return self.vault("remove", keys)

    def vault_retrieve(self, keys=list()) -> tuple:
        return self.vault("retrieve", keys)

    def vault_retrieve_keys(self, keys=list()) -> tuple:
        return self.vault("retrieveKeys", keys)

    def vault_update(self, keys=list(), data=dict()) -> tuple:
        return self.vault("update", keys, data)
