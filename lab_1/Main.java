import java.util.ArrayList;
import java.util.Arrays;

public class Main {
  
  public static void main(String[] args) {
    Monitor monitor = new Monitor();
    Provider provider = new Provider(monitor);
    Consumer consumer = new Consumer(monitor);
    new Thread(provider).start();
    new Thread(consumer).start();
  }
}

class Monitor{

  ArrayList<Integer> departure = new ArrayList<Integer>(Arrays.asList(1, 2, 3, 4 , 5, 6, 7, 8, 9, 10));
  ArrayList<Integer> arrival = new ArrayList<Integer>();
  int hub;
  boolean ready = false;

  public synchronized void provide(){
    while(!departure.isEmpty()){
      while(ready){
        try{
          wait();
        }
        catch(InterruptedException e){}
      }
      hub = departure.get(0);
      departure.remove(0);
      System.out.println("Send: " + hub);
      ready = true;
      notifyAll();
    }
  }

  public synchronized void consume(){
    while(!departure.isEmpty()){
      while(!ready){
        try{
          wait();
        }
        catch(InterruptedException e){}
      }
      arrival.add(hub);
      System.out.println("Receive: " + hub);
      ready = false;
      notifyAll();
    }
  }

}

class Provider implements Runnable{

  Monitor monitor;

  Provider(Monitor monitor){
    this.monitor = monitor;
  }

  public void run(){
    monitor.provide();
  }
}

class Consumer implements Runnable{

  Monitor monitor;

  Consumer(Monitor monitor){
    this.monitor = monitor;
  }
  
  public void run(){
    monitor.consume();
  }
}