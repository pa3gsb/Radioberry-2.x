package org.radioamateur.sdr;

import org.zeromq.SocketType;
import org.zeromq.ZContext;
import org.zeromq.ZMQ;

public class hwclient {

    public static void main(String[] args)
    {
        try (ZContext context = new ZContext()) {
            //  Socket to talk to server
            System.out.println("Connecting to hello world server");

            ZMQ.Socket socket = context.createSocket(SocketType.REQ);
            socket.connect("tcp://192.168.2.140:5555");

//            String request = "Hello";
// /           System.out.println("Sending Hello ");
//            socket.send(request.getBytes(ZMQ.CHARSET), 0);
//

            for (int requestNbr = 0; requestNbr != 1000000; requestNbr++) {
                String request = "Hello";
                System.out.println("Sending Hello " + requestNbr);
                socket.send(request.getBytes(ZMQ.CHARSET), 0);
//
//                try{
//                    Thread.sleep(1000); //  Do some 'work'
//                } catch(Exception ex) {ex.getMessage();}

                byte[] reply = socket.recv(0);
                System.out.println("length of reply = " + reply.length);
//                System.out.println(
//                        "Received " + new String(reply, ZMQ.CHARSET) + " " +
//                                requestNbr
//                );
            }
        }
    }
}
