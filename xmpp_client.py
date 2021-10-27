#!/usr/bin/python
# Copyright (c) Twisted Matrix Laboratories.
# See LICENSE for details.

"""
A very simple twisted xmpp-client (Jabber ID)

To run the script:
$ python xmpp_client.py <jid> <secret>
"""

from __future__ import print_function

import sys

from twisted.internet.defer import Deferred
from twisted.internet.task import react
from twisted.names.srvconnect import SRVConnector
from twisted.words.xish import domish
from twisted.words.protocols.jabber import xmlstream, client
from twisted.words.protocols.jabber.jid import JID


rot13 = str.maketrans(
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
    "nopqrstuvwxyzabcdefghijklmNOPQRSTUVWXYZABCDEFGHIJKLM"
)


class Client(object):
    def __init__(self, reactor, jid, secret):
        self.reactor = reactor
        f = client.XMPPClientFactory(jid, secret)
        f.addBootstrap(xmlstream.STREAM_CONNECTED_EVENT, self.connected)
        f.addBootstrap(xmlstream.STREAM_END_EVENT, self.disconnected)
        f.addBootstrap(xmlstream.STREAM_AUTHD_EVENT, self.authenticated)
        f.addBootstrap(xmlstream.INIT_FAILED_EVENT, self.init_failed)
        connector = SRVConnector(
            reactor, 'xmpp-client', jid.host, f, defaultPort=5222)
        connector.connect()
        self.finished = Deferred()


    def rawDataIn(self, buf):
#       print(buf.decode())
        pass


    def rawDataOut(self, buf):
#       print(buf.decode())
        pass


    def connected(self, xs):
        print('Connected.')

        self.xmlstream = xs

        # Log all traffic
        xs.rawDataInFn = self.rawDataIn
        xs.rawDataOutFn = self.rawDataOut

        xs.addObserver('/presence[@type="subscribe"]', self.handle_subscribe)
        xs.addObserver('/message[@type="chat"]', self.handle_chat)


    def disconnected(self, xs):
        print('Disconnected.')

        self.finished.callback(None)


    def authenticated(self, xs):
        print("Authenticated.")

        presence = domish.Element((None, 'presence'))
        xs.send(presence)

#       self.reactor.callLater(5, xs.sendFooter)


    def init_failed(self, failure):
        print("Initialization failed.")
        print(failure)

        self.xmlstream.sendFooter()

    def handle_subscribe(self, e):
        self.xmlstream.send(domish.Element(
            (None, 'presence'),
            attribs={
                'to': e['from'],
                'type': 'subscribed',
            }
        ))
        self.xmlstream.send(domish.Element(
            (None, 'presence'),
            attribs={ 
                'to': e['from'],
                'type': 'subscribe',
            }
        ))
        print("Subscribe from: {}".format(e['from']))

    def handle_chat(self, e):
        for c in e.children:
            if c.name == 'body':
                break
        else:
            return

        response = domish.Element(
            ('None', 'message'),
            attribs={
                'to': e['from'],
                'type': 'chat',
                'from': e['to'],
            },
        )

        msg = str(c)
        print("from={} msg={}".format(e['from'], msg))
        body = domish.Element(('None', 'body'))
        body.addContent(msg.translate(rot13))
        response.addChild(body)
        self.xmlstream.send(response)


def main(reactor):
    """
    Connect to the given Jabber ID and return a L{Deferred} which will be
    called back when the connection is over.

    @param reactor: The reactor to use for the connection.
    @param jid: A L{JID} to connect to.
    @param secret: A C{str}
    """

    jid = 'bot@example.net'
    secret = 'password'
    return Client(reactor, JID(jid), secret).finished


if __name__ == '__main__':
    react(main)
