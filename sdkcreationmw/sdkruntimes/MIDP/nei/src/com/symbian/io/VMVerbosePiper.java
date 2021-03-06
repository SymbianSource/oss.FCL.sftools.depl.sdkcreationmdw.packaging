/*
* Copyright (c) 2003 - 2004 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/

package com.symbian.io;

import java.net.*;
import java.io.*;
import javax.swing.JOptionPane;

import com.symbian.utils.Debug;
import com.symbian.utils.SystemExitHandler;
import com.symbian.vm.VMProcess;


/**
 * Thread to connect to the VM console port and pipe all output prints.
 * The VM will receive a command line argument "-verboseport [port]", open a
 * server socket on that port and send all output.
 *
 * Another usage for this thread is to notice if the VM exited. this is required
 * when the VM process is running on a remote target. when the connection is e
 *
 * <p>Title: PREQ 425</p>
 * <p>Description: Java debugging</p>
 * <p>Copyright: Copyright (c) 2004 By Symbian Ltd. All Rights Reserved</p>
 * <p>Company: Symbian Ltd</p>
 * @author Roy Ben Hayun
 * @version 2.0
 */
public class VMVerbosePiper extends Thread {

  //
  // Constants
  //

  /**
   * Threads name
   */
  private static final String NAME = "VMVerbosePiper-";




  //
  // Members
  //

  /**
   * Interval to wait between connection attempts
   */
  private static int sInterval = 150;

  /**
   * Threads counters
   */
  private static int cCounter = 0;

  /**
   * VM host
   */
  private InetAddress iVMAddress;

  /**
   * VM port
   */
  private int iVMVerbosePort;

  /**
   * Print stream (typically, System.out)
   */
  private PrintStream iPrintStream;

  /**
   * The VM process.
   */
  private VMProcess iVMProcess;

  /**
   * Socket to verbose port
   */
  private Socket iSocket;

  /**
   * Number of connection attempts
   */
  private int iAttempts;






  //
  // Life cycle
  //

  /**
   * Constructor.
   *
   * @param aVMAddress VM address
   * @param aVMVerbosePort VM verbose listening port
   * @param aPrintStream print stream
   * @param aProcess VM process
   */
  public VMVerbosePiper(InetAddress aVMAddress,
                        int aVMVerbosePort,
                        PrintStream aPrintStream,
                        VMProcess aProcess,
                        int aAttempts) {
    super(NAME+ cCounter++);
    iVMAddress = aVMAddress;
    iVMVerbosePort = aVMVerbosePort;
    iPrintStream = aPrintStream;
    iVMProcess = aProcess;
    iAttempts = aAttempts;
  }

  //
  // Operations
  //

  private void waitForConnection(){
    Debug.println(this, "+VMVerbosePiper.waitForConnection");
    //boolean connected = false;
    //int attempts = 0;
    try{
    	ServerSocket serverSocket = new ServerSocket(iVMVerbosePort);
    	iSocket = serverSocket.accept();
    }
    catch(Exception e) {
    	Debug.println(this, "VMVerbosePiper error on listening IO port: "+e.getMessage());
    }
    
    
  /*  while(!connected){
      try {
        attempts++;
        Debug.println(this, "VMVerbosePiper attempts to connect to " + iVMAddress.getHostName() + ":" + iVMVerbosePort);
        iSocket = new Socket(iVMAddress.getHostName(), iVMVerbosePort);
        connected = true;
        iVMProcess.setAttachedAndAlive();
        Debug.println(this, "VMVerbosePiper connected to VM console");
      }
      catch(IOException e) {
        if(attempts > iAttempts){
          if(JOptionPane.showConfirmDialog
              (null,
               "Would you like to wait more?",
               "Operation timed out",
               JOptionPane.YES_NO_OPTION,
               JOptionPane.INFORMATION_MESSAGE)
              != JOptionPane.YES_OPTION){
             SystemExitHandler.invokeSystemExit(-51);
           }
           attempts = 0;
        }
        Debug.println(this, "VMVerbosePiper connection failed. wait some more..");
        try {
          synchronized (this) {
            wait(sInterval);
          }
        }
        catch(Exception e2) {}
      }
    }*/
  }

  /**
   * Threads target
   */
  public void run(){
    try {
      waitForConnection();
      //alternative to pipe single characters is use Nagel's algorithm
      final InputStream is = iSocket.getInputStream();
      int read = is.read();
      while(read != -1){
        iPrintStream.print((char)read);
        read = is.read();
      }
    }
    catch (Exception e) {
      Debug.printStackTrace(this, e);
    }
    finally{
      System.out.println("-VM verbose connection exited");
      Debug.println(this, "VM verbose connection exited");
      closeConnection();
      iVMProcess.setAlive(false);
      SystemExitHandler.invokeSystemExit(0);
    }
  }

  /**
   * Brute force close the socket
   */
  public void closeConnection()
  {
    Debug.println(this, "+VMVerbosePiper.closeConnection");
    if (iSocket != null)
    {
      try
      {
        iSocket.close();
      }
      catch (Exception e)
      {
        Debug.printStackTrace(this, e);
      }
      iSocket = null;
    }
    Debug.println(this, "-VMVerbosePiper.closeConnection");
  }

}
