package com.synel.synergy.synergy2416.presentation.api;

public interface FingerPrintEnrollmentHandler {
	/**
	 * Sets the number of times user has to present the finger to reader
	 * @param count Number of times user is asked to place fingers
	 */
	
	 public void setStepCount(int count);
	 
	 /**
	  * Prepares for finger print image reading. Could prompt user to place finger with a message appropriate to the current step 
	  * @param step  Finger print reading attempt from  0 to the value set in number of steps
	  * @param repeatOnReaderError Flag to indicate if this is because of error in the previous attempt
	  * @see #setStepCount(int)
	  */
	 
	  public void onReadyForFinger(int step, boolean repeatOnReaderError);
	    
	  /**
	   * Handles the availability of finger print image.  This is invoked after the finger print image is read by the reader. User could be prompted to remove finger from the reader.  
	   **/
	  
	  public void onFingerPrintRead(int step);

}