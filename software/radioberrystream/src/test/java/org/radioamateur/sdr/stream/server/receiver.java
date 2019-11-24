package org.radioamateur.sdr.stream.server;

import java.util.Random;

import org.zeromq.SocketType;
import org.zeromq.ZMQ;
import org.zeromq.ZContext;

public class receiver {

    public static void main(String[] args) throws Exception
    {
        //  Prepare our context and publisher
        try (ZContext context = new ZContext()) {
            ZMQ.Socket publisher = context.createSocket(SocketType.PUB);
            publisher.bind("tcp://*:5556");
            publisher.bind("ipc://weather");

            //  Initialize random number generator
            Random srandom = new Random(System.currentTimeMillis());
            while (!Thread.currentThread().isInterrupted()) {
                //  Get values that will fool the boss
                int zipcode, temperature, relhumidity;
                zipcode = 10000 + srandom.nextInt(10000);
                temperature = srandom.nextInt(215) - 80 + 1;
                relhumidity = srandom.nextInt(50) + 10 + 1;

                //  Send message to all subscribers
                String update = String.format(
                        "%05d %d %d", zipcode, temperature, relhumidity
                );
                publisher.send(update, 0);
            }
        }
    }




}
