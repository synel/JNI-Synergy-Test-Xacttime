/**
 * Copyright (C) 2013-2014 Time America Inc.
 * @author chaol
 */

package com.synel.synergy.synergyX.presentation.controller;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Random;
import java.util.logging.Logger;

import com.synel.synergy.synergyX.presentation.api.FingerPrintEnrollmentHandler;

public final class FPU implements FingerPrintEnrollmentHandler, Runnable{
	private static Logger logger = Logger.getLogger(FPU.class.getName());
	private static boolean isMock = false;
	private static boolean m_bFingerisEnrolling = false;
	private static boolean m_bFingerprintValidating = false;
	private static boolean m_bFingerprintIdentifing = false;
	private static boolean m_allowIdentifyEmployee = false;

	/**
	 * @return the m_allowIdentifyEmployee
	 */
	public static boolean is_allowIdentifyEmployee() {
		return m_allowIdentifyEmployee;
	}

	/**
	 * @param m_allowIdentifyEmployee
	 *            the m_allowIdentifyEmployee to set
	 */
	public static void set_allowIdentifyEmployee(boolean allowIdentifyEmployee) {
		FPU.m_allowIdentifyEmployee = allowIdentifyEmployee;
	}

	static {
		logger.info("You are running on: "+System.getProperty("os.arch")+" System"); //$NON-NLS-1$
		if (!System.getProperty("os.arch").contains("arm")) { //$NON-NLS-1$ //$NON-NLS-2$
			isMock = true;
		} else {
			logger.info("open library...");
			System.loadLibrary("SynergyX"); //$NON-NLS-1$
		}
	}

	private static class FPULoader {
		private static final FPU INSTANCE = new FPU();
	}

	private FPU() {
		if (FPULoader.INSTANCE != null) {
			throw new IllegalStateException("Already instantiated"); //$NON-NLS-1$
		}
	}

	public static FPU getInstance() {
		return FPULoader.INSTANCE;
	}

	private static native void REDON();

	private static native void REDOFF();

	private static native void GREENON();

	private static native void GREENOFF();

	/**
	 * Opens The FPU DEVICE
	 * 
	 * @param templateFolder
	 *            Templates are stored and loaded from a folder specified.
	 *            Templates are stored with the following
	 *            %Badge_%FingerNum.template
	 * @return 0 as a long if successful anything else means the it failed to
	 *         open
	 */

	private static native int FP_OPENDEVICE(String templateFolder);

	/**
	 * Closes The FPU Device
	 * 
	 * @return 0 as a long if successful anything else means the it failed to
	 *         close
	 */
	private static native int FP_CLOSEDEVICE();

	/**
	 * Enrolls a new employee.. No visual/audio feed back to the user. In order
	 * to enroll an employee the finger must be presented to the reader and with
	 * drawn from the reader 3 times. IE enrollment is called- user is prompted
	 * to place finger. finger is placed. finger is withdrawn for ~1-2 seconds.
	 * User is prompted to place finger again. Finger is placed. finger is
	 * withdrawn for ~1-2 seconds. finger is placed a third and last time.
	 * Enrollment is complete.
	 * 
	 * @param badge
	 *            Badge number of the employee
	 * @param fingerNum
	 *            is the number of the finger and should not be < 0 or > 9 (0-9)
	 * @param timeOut
	 *            is the length of time the finger print reader will wait for a
	 *            finger to be presented
	 * @param gapTime
	 *            The amount of time until the next onReadyForFinger Call back
	 *            is made after a successful Read;
	 * @param enrollmentHandler
	 *            is the Java call back interface.
	 * @return <li><code>-103</code> Bad Finger Read - The Finger print sensor
	 *         failed to pickup a finger or the image was distorted or
	 *         unreadable. This can sometimes be caused by a lack of moisture on
	 *         the finger or a foreign substance that obstructs the read</li>
	 *         <li><code>-107</code> Bad Finger Read or No finger Present - The
	 *         Finger print sensor failed to pickup a finger or the image was
	 *         distorted or unreadable. This can sometimes be caused by a lack
	 *         of moisture on the finger or a foreign substance that obstructs
	 *         the read. Or the sensor has no finger on it</li> <li>
	 *         <code>-108</code> There are too many templates in the finger
	 *         print reader to begin enrollment. Each fingerprint reader can
	 *         only support a certain number of templates. Usually 3,000 or
	 *         10,000. Consult with the manufacturer if this problem arises and
	 *         you believe you are under the proper threshold.</li> <li>
	 *         <code>-109</code> Bad Badge, The badge you entered is not
	 *         supported.</li> <li><code>-106</code> The finger you are trying
	 *         to Enroll already exists in the finger print reader under a
	 *         different badge. You must first delete this finger under the
	 *         corresponding badge if you wish to enroll this finger under a new
	 *         badge. You cannot have one finger associated with 2 different
	 *         badges.</li> <li><code>-110</code> Finger print template failed
	 *         to save to file. Be sure finger print folder location exists and
	 *         that you have the proper permission to write to this folder. This
	 *         method will not create the folder if it does not exists the
	 *         folder location must exist already. <li><code>0</code> Enrollment
	 *         Successful</li>
	 **/
	private static native int FP_ENROLE_EMPLOYEE(String uid, int fingerNum,
			long timeOut, long gapTime,
			FingerPrintEnrollmentHandler enrollmentHandler);

	/**
	 * Validates whether the employee badge is enrolled in the reader
	 * 
	 * @param badge
	 *            Badge Number of Employee
	 * @param fingerNum
	 *            the finger number of the employee
	 * @return <li><code>-102</code> Badge Number Supplied not found - The Badge
	 *         Number was not found in the fingerprint database. The badge
	 *         entered does not correspond to any badge in the reader</li> <li>
	 *         <code>0</code> Badge exists in finger print reader.
	 */
	private static native int FP_GET_BADGE_STATUS(String uid, int fingerNum);

	/**
	 * Validates finger print of an employee identified by badge. No
	 * visual/audio feed back to the user.
	 * 
	 * @param badge
	 *            Badge number of the employee to validate
	 * @param fingerNum
	 *            Finger to be used for validation
	 * @return <li><code>-101</code> No Templates in Reader - There are no user
	 *         templates in the reader the enroll count is 0 and cannot validate
	 *         the badge</li> <li><code>-102</code> Badge Number Supplied not
	 *         found - The Badge Number was not found in the fingerprint
	 *         database. The badge entered does not correspond to any badge in
	 *         the reader</li> <li><code>-103</code> Bad Finger Read - The
	 *         Finger print sensor failed to pickup a finger or the image was
	 *         distorted or unreadable. This can sometimes be caused by a lack
	 *         of moisture on the finger or a foreign substance that obstructs
	 *         the read</li> <li><code>-104</code> Finger did not match - The
	 *         badge was found but the finger supplied for validation did not
	 *         match to the template in the finger print sensor</li> <li>
	 *         <code>0</code> Badge and Finger Validated
	 */
	private static native int FP_VALIDATE_EMPLOYEE(String uid, int fingerNum,
			long timeOut, int isSpecialEnrolled);

	/***
	 * Deletes finger print template(s) of an employee.
	 * 
	 * @param badge
	 *            Badge number of the employee
	 * @param fingerNum
	 *            Finger number of the employee template. A negative value means
	 *            all finger prints of the employee
	 * @return <li><code>0</code> successfully removed the requested finger
	 *         print(s)</li> <li><code>-101</code> No Templates in Reader -
	 *         There are no user templates in the reader the enroll count is 0
	 *         and cannot delete the badge</li> <li><code>-102</code> Badge
	 *         Number Supplied/finger number not found - The Badge Number/finger
	 *         number was not found in the fingerprint database. The badge
	 *         entered does not correspond to any badge in the reader</li>
	 */
	private static native int FP_DELETE_TEMPLATE(String uid, int fingerNum);

	/**
	 * Gets all known badges from the finger print reader
	 * 
	 * @return A <code>java.lang.String</code> array containing all known
	 *         badges. Can't be null.
	 */
	private static native String[] FP_GET_BADGES();

	/**
	 * Gets current number of templates in the finger print sensor
	 * 
	 * @return A <code>long</code> Number of templates currently in the finger
	 *         print sensor
	 */

	private static native int FP_GET_ENROLECOUNT();

	/**
	 * Gets a finger print template of an employee
	 * 
	 * @param badge
	 *            Badge number of an employee
	 * @param fingerNum
	 *            Finger number of the employee
	 * @return Returns template if it exists or null if it does not exist
	 */
	private static native String FP_GET_TEMPLATE(String uid, int fingerNum);

	/**
	 * Loads a base64 encoded representation of a finger template. Then saves
	 * the template to a file in a %badge_%fingerNum.template file in the folder
	 * given by the FP_OPEN_DEVICE method New template will replace if a
	 * template already exists.
	 * 
	 * @param badge
	 *            Badge number of an employee whose template is loaded
	 * @param fingerNum
	 *            Finger number of the employee
	 * @param template
	 *            Finger print template in a base64 encoded string to load
	 * @return Returns the status of the action <li><code>0</code> Template was
	 *         loaded into memory and the file was created successfully <li>
	 *         <code>-100</code> The data supplied to the function. Badge
	 *         Number, finger number or Template format is incorrect. The
	 *         template was not enrolled and not saved to a file. <li>
	 *         <code>-110</code> Finger print template failed to save to file.
	 *         Be sure finger print folder location exists and that you have the
	 *         proper permission to write to this folder. This method will not
	 *         create the folder if it does not exists the folder location must
	 *         exist already. <li>An error message if failed to load</li>
	 */
	private static native int FP_SET_TEMPLATE(String uid, int fingerNum,
			String template);

	/**
	 * Identifies finger print of an employee. User presents finger and finger
	 * is mapped to a badge. The badge that corresponds to the finger is
	 * returned. No visual/audio feed back to the user.
	 * 
	 * @return <li><code>-101</code> No Templates in Reader - There are no user
	 *         templates in the reader the enroll count is 0 and cannot validate
	 *         the badge</li> <li><code>-105</code> Finger did not match - The
	 *         supplied finger did not match any template in the finger print
	 *         sensor</li> <li><code>-107</code> Bad Finger Read or No finger
	 *         Present - The Finger print sensor failed to pickup a finger or
	 *         the image was distorted or unreadable. This can sometimes be
	 *         caused by a lack of moisture on the finger or a foreign substance
	 *         that obstructs the read. Or the sensor has no finger on it</li>
	 *         <li><code>long</code> badge Number > 0. Finger Identified
	 *         corresponds to the badge number returned by this function. Will
	 *         always be greater than 0. Success.
	 */

	private static native int FP_IDENTIFY_EMPLOYEE();

	/***
	 * Deletes all finger print template(s) in the FPU
	 * 
	 * @return <li><code>0</code> successfully removed all finger print(s)</li>
	 *         <li><code>-101</code> No Templates in Reader - There are no user
	 *         templates in the reader the enroll count is 0 and cannot delete
	 *         the badge</li>
	 */

	private static native int FP_DELETE_TEMPLATE_ALL();

	private static native int FP_IDENTIFY_EMPLOYEE_MOCK(String badgeNum);

	private static String app_msgConvert(int nRet) {
		switch (nRet) {
		case 0:
			return "Success"; //$NON-NLS-1$
		case -1:
			return "FPU identification function yield to others"; //$NON-NLS-1$
		case -100:
			return "The data supplied to the function, Badge Number, finger number or Template format is incorrect."; // The //$NON-NLS-1$
		case -101:
			return "No templates in FPU reader"; //$NON-NLS-1$
		case -102:
			return "Badge number not Found"; //$NON-NLS-1$
		case -103:
			return "Bad finger Read"; //$NON-NLS-1$
		case -104:
			return "Finger did not match badge number"; // - The badge was found //$NON-NLS-1$
		case -5:
		case -105:
			return "Finger did not match"; //$NON-NLS-1$
		case -106:
			return "Finger already enrolled";// "The finger you are trying to Enroll already exists in the finger print reader under a different badge."; //$NON-NLS-1$
		case -107:
			return "Bad finger read or No finger present"; //$NON-NLS-1$
		case -108:
			return "Reached max number of fp templates allowed in FPU"; // "There are too many templates in the finger print reader to begin enrollment.";// //$NON-NLS-1$
		case -109:
			return "Bad badge, The badge you entered is not supported"; //$NON-NLS-1$
		case -110:
			return "Finger print template failed to save to file"; //$NON-NLS-1$
		case -111:
			return "The Enrollment function failed to return an error code"; //$NON-NLS-1$
		case -999:
		case -8:
			return "No Finger Pressed"; //$NON-NLS-1$
		default:
			return Integer.toString(nRet);
		}
	}

	public enum Light {
		RED, GREEN;
		public void on() {
			if (this.compareTo(RED) == 0) {
				if(isMock) {
					System.out.println("Red ON");
				} else {
					REDON();
				}
			} else {
				if(isMock) {
					System.out.println("Green ON");
				} else {
					GREENON();
				}
			}
		}

		public void off() {
			if (this.compareTo(RED) == 0) {
				if(isMock) {
					System.out.println("Red OFF");
				} else {
					REDOFF();
				}
			} else {
				if(isMock) {
					System.out.println("Green OFF");
				} else {
					GREENOFF();
				}
			}
		}
	}

	public static int openFPU(String temploc) {
		if(!isMock) {
			return FPU.FP_OPENDEVICE(temploc);
		} else {
			return 0;
		}
	}

	public static String closeFPU() {
		if(!isMock){
			return app_msgConvert(FPU.FP_CLOSEDEVICE());
		}else {
			return "fpu closed";
		}
	}

	public static String enroll(String strUID, int nFingerNum,
			FingerPrintEnrollmentHandler fph) {
		logger.info("Enrolling " + strUID + " FingerNum: " + nFingerNum); //$NON-NLS-1$ //$NON-NLS-2$
		int enrollMessage = -111;
		m_bFingerisEnrolling = true;
		logger.info("setting fp enrollment to " + m_bFingerisEnrolling); //$NON-NLS-1$
		enrollMessage = FPU.FP_ENROLE_EMPLOYEE(strUID, nFingerNum, 15000,
				1000, fph);
		m_bFingerisEnrolling = false;
		logger.info("setting fp enrollment to " + m_bFingerisEnrolling); //$NON-NLS-1$
		return app_msgConvert(enrollMessage);
	}

	public static String encodedTemplate(String strBadgeNum, int nFingerNum) {
		return FPU.FP_GET_TEMPLATE(strBadgeNum, nFingerNum);
	}

	public static String enrollCount() {
		return app_msgConvert(FPU.FP_GET_ENROLECOUNT());
	}

	public static String validateEmployee(String strBadgeNum, int nFingerNum) {
		m_bFingerprintValidating = true;
		logger.info("running validating employee..."); //$NON-NLS-1$
		int res = FPU.FP_VALIDATE_EMPLOYEE(strBadgeNum, nFingerNum, 15000, 0);
		m_bFingerprintValidating = false;
		return app_msgConvert(res);
	}

	public static String validateEmployee(String strUID, int nFingerNum,
			boolean isSpecialEnrolled) {
		m_bFingerprintValidating = true;
		logger.info("running validating special employee..."); //$NON-NLS-1$
		int res = FPU.FP_VALIDATE_EMPLOYEE(strUID, nFingerNum, 15000,
				isSpecialEnrolled ? 1 : 0);
		m_bFingerprintValidating = false;
		return app_msgConvert(res);
	}

	public static String badgeStatus(String strUID, int nFingerNum) {
		return app_msgConvert(FPU.FP_GET_BADGE_STATUS(strUID, nFingerNum));
	}

	public static String deleteFPTemplate(String strBadgeNum, int nFingerNum) {
		return app_msgConvert(FPU.FP_DELETE_TEMPLATE(strBadgeNum, nFingerNum));
	}

	public static String deleteAllFPTemplate() {
		return app_msgConvert(FPU.FP_DELETE_TEMPLATE_ALL());
	}

	public static String[] getAllUIDs() {
		if (isMock) {
			return new String[] { "101", "102", "103", "104", "105", "106", //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$ //$NON-NLS-5$ //$NON-NLS-6$
					"109", "108", "107" }; //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
		}
		return FPU.FP_GET_BADGES();
	}

	public static String getFPTemplate(String strUID, int nFingerNum) {
		return FPU.FP_GET_TEMPLATE(strUID, nFingerNum);
	}

	public static String setFPTemplate(String strUID, int nFingerNum,
			String template) {
		return app_msgConvert(FPU.FP_SET_TEMPLATE(strUID, nFingerNum,
				template));
	}

	public static String identifyEmployee() {
		int nRet = -107;
		while (nRet == -107 || nRet == -101) {
			try {
				Thread.sleep(400);
			} catch (InterruptedException e) {
				logger.info(e.getLocalizedMessage());
			}
			// logger.debug("allowIdentifyingEmployee is "+m_allowIdentifyEmployee);
			if (m_allowIdentifyEmployee
					&& (!m_bFingerisEnrolling || !m_bFingerprintValidating)) {
				if (!isMock) {
					m_bFingerprintIdentifing = true;
					logger.info("running identify employee...");
					nRet = FPU.FP_IDENTIFY_EMPLOYEE();
					m_bFingerprintIdentifing = false;
				} else {
					// nRet = FPU.FP_IDENTIFY_EMPLOYEE_MOCK(m_badgenum);
					nRet = Integer.parseInt("501"); //$NON-NLS-1$
					logger.info("identifying employee: 501"); //$NON-NLS-1$
				}

			} else {
				// other thread require to run give away right of way
				return "threadCancelled"; //$NON-NLS-1$
			}
		}
		return app_msgConvert(nRet);
	}

	public static void main(String[] args) {
		System.out.println("Java Lib Path is: "+System.getProperty("java.library.path")); //$NON-NLS-1$ //$NON-NLS-2$
		FPU.openFPU("/tmp"); //invoke the help msg for this library
		FPU.getInstance().REDOFF();
		FPU.getInstance().GREENOFF();
		System.out.println("GREEN ON:"); //$NON-NLS-1$
		FPU.getInstance().GREENON();
		try {
			Thread.sleep(3000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		System.out.println("RED OFF:"); //$NON-NLS-1$
		FPU.getInstance().REDOFF();
		try {
			Thread.sleep(1500);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		System.out.println("GREEN OFF:"); //$NON-NLS-1$
		FPU.getInstance().GREENOFF();

		try {
			Thread.sleep(3000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		String Template="";
		int exit = 1;
		while(exit!=0){
			System.out.print("Choose: \n 0) Identify Employee \n 1) Enroll \n 2) Delete \n 3) Validate \n 4) Enroll Count\n 5) Get Template \n 6) Set Template \n 7) Get Templates \n 8) Green Toggle \n 9) Red Toggle\n 10) Thread Toggle\n 11) exit \n Choice:\n");
			switch(selectionChoice()){
			case 0:
				System.out.println("identify result : "+identifyEmployee());
				break;
			case 1:
				System.out.println("Enrollment Status: "+FP_ENROLE_EMPLOYEE(stringChoice("Enter Employee Number:"),Integer.parseInt(stringChoice("Enter Finger Number:")),(long)15000,(long)1000,FPU.getInstance()));
				break;
			case 2:
				System.out.println("Delete Template: "+FP_DELETE_TEMPLATE(stringChoice("Enter Employee Number:"), Integer.parseInt(stringChoice("Enter Finger Number:"))));
				break;
			case 3:
				System.out.println(FP_VALIDATE_EMPLOYEE(stringChoice("Enter Employee Number:"),Integer.parseInt(stringChoice("Enter Finger Number:")),(long)15000, 0));
				break;
			case 4:
				System.out.println("Enroll Count: "+FP_GET_ENROLECOUNT());
				break;
			case 5:
				Template = FP_GET_TEMPLATE(stringChoice("Enter Employee Number:"), Integer.parseInt(stringChoice("Enter Finger Number:")));
				System.out.println(Template);
				break;
			case 6:
				for (int i =100; i< 130; i++)
				{
					String empNbr = "101";
					//int fingerNbr = 101;
					int min =110, max=120;
					Random randomNum = new Random();
					int fingerNbr = min +randomNum.nextInt(max);
					System.out.println("Set Template: " +FP_SET_TEMPLATE(empNbr, fingerNbr, Template));
				}
				//System.out.println("Set Template: " +FP_SET_TEMPLATE(stringChoice("Enter Employee Number:"), Integer.parseInt(stringChoice("Enter Finger Number:")), Template));
				break;
			case 7:
				String [] badges = null;
				badges =  FP_GET_BADGES();
				for (String s: badges){
					System.out.println(s);
				}
				break;
			case 8:
				System.out.println("Toggle Green");
				GREENON();
				try {
					Thread.sleep(3000);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				GREENOFF();
				break;
			case 9:
				System.out.println("Toggle RED");
				REDON();
				try {
					Thread.sleep(3000);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				REDOFF();
				break;
			case 10:
				System.out.println("Thread Selected");
				(new Thread(FPU.getInstance())).start();
				break;
			default:
				exit = 0;

			}
		}
	}

	public void setStepCount(int count) {
		System.out.println("This is the count you recieved: "+count);

	}

	@Override
	public void onReadyForFinger(int step, boolean repeatOnReaderError) {
		switch(step){
		case 1:
			System.out.println("Please place finger Step: "+ step+ "Error: " + repeatOnReaderError);
			break;
		case 2:
			System.out.println("Please place finger Step: "+ step+ "Error: " + repeatOnReaderError);
			break;
		case 3:
			System.out.println("Please place finger Step: "+ step+ "Error: " + repeatOnReaderError);
			break;
		}


	}
	@Override
	public void onFingerPrintRead(int step) {
		System.out.println("Please Remove Finger Step: "+ step);
	}

	public static int selectionChoice(){
		int choice =0;
		String input = "";
		try{
			BufferedReader br =
					new BufferedReader(new InputStreamReader(System.in));
			input=br.readLine();
		}
		catch (Exception e){
			e.printStackTrace();
		}
		choice= Integer.parseInt(input);
		System.out.println("You Chose: "+ choice);
		return choice;
	}
	
	public static String stringChoice(String prompt){
		String input = "";
		System.out.print(prompt);
		try{
			BufferedReader br =
					new BufferedReader(new InputStreamReader(System.in));

			input=br.readLine();

		}
		catch (Exception e){
			e.printStackTrace();
		}
		return input;
	}
	public void run(){
		System.out.println("Thread Start");
		try {
			Thread.sleep(2000);
			Runtime.getRuntime().exec("echo Hello World");
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		System.out.println("Thread Stop");
	}

}
