package org.radioamateur.sdr;

import org.zeromq.SocketType;
import org.zeromq.ZContext;
import org.zeromq.ZMQ;

public class wuclient {

    public static void main(String[] args)
    {
        try (ZContext context = new ZContext()) {
            //  Socket to talk to server
            ZMQ.Socket subscriber = context.createSocket(SocketType.SUB);
            subscriber.connect("tcp://192.168.2.140:5556");

            //String filter = (args.length > 0) ? args[0] : "10001 ";
            //filter.getBytes(ZMQ.CHARSET)
            String filter = "rx0";
            subscriber.subscribe(filter.getBytes(ZMQ.CHARSET));

            while(true) {
                byte[] iqdata = subscriber.recv(0);

                //System.out.println("iq lengte" + iqdata.length );
                byte[] recv = new byte[3];
                System.arraycopy(iqdata,0,recv, 0, 3);
                String s = new String(recv);
                System.out.println(s);



            }
        }
    }

}
