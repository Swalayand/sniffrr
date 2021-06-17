import java.net.*;
import java.io.*;

public class SocketClient {

  public static void main(String[] args){
    // String HOST_ADDRESS = "127.0.0.1";
    // int PORT = 8080;
    String HOST_ADDRESS = "192.168.4.1";
    int PORT = 80;
    
    try{
      BufferedReader bufferedReader = null;
      // PrintWriter printWriter = null;
      Socket socket = new Socket(HOST_ADDRESS, PORT);
      Boolean loop = true;
      int i = 0;
      while(i < 10){
        bufferedReader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        System.out.println(bufferedReader.readLine());
        // printWriter = new PrintWriter(socket.getOutputStream());
        // printWriter.write("Hello user!\n");
        // printWriter.flush();
        //socket.close();
        i+=1;
      }
      bufferedReader.close();
      // printWriter.close();
    }catch(IOException e){}
  }

}