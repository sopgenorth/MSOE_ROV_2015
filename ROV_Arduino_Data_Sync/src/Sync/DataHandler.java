package sync;

import java.awt.event.ActionListener;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Timer;
import java.util.TimerTask;

import hypermedia.net.*;

/**
 * <p>A class used to synchronize data with a connected device using UDP streams</p>
 * 
 * <p>Usage Guide:</p>
 * <dl>
 * <dt>Instantiating</dt>
 * <dd>Instantiate using the settings you need, many values can be left to their
 * default settings. Will schedule event calls to update self</dd>
 * <dt>Usage</dt>
 * <dd>Use getValue and setValue to access and change the data stored, setValue
 * will change the data which will be sent to the device automagically</dd>
 * <dt>Error Handling</dt>
 * <dd>setDeviceTimeoutListener can be used to alert your program when
 * connection with the device is lost</dd>
 * </dl>
 * 
 * @author millerds
 *
 */
public class DataHandler {

	private class Updater extends TimerTask {
		@Override
		public void run() {
			if (timeout > 0) {
				if (DataHandler.this.getTimeSinceLastUpdate() > timeout) {
					timeoutAction.actionPerformed(null);
					print("Triggering timeout action");
					timeout = -1;
				}
			}
			updateOutNums();
		}

	}

	private UDP udp; // define the UDP object
	private String deviceIP;
	private int devicePort;
	private boolean debug;

	private int[] inNums;
	private int[] outNums;
	private String[] inNames;
	private String[] outNames;

	/**
	 * Keeps track of the last time the arduino sent an update
	 */
	private long lastUpdate;

	private ActionListener timeoutAction;
	private long timeout;

	private Timer timer;

	/**
	 * Start synchronizing with the device
	 * 
	 * @param inNames
	 *            Array of names for variables to receive from the device
	 * @param outNames
	 *            Array of names for variables to be sent to the device
	 * @param updateFrequency
	 *            Ms between updates sent to device. default is 100ms
	 * @param debugPrint
	 *            Debug print switch. default is false
	 * @param deviceIP
	 *            IP of device. Default is 192.168.1.117
	 * @param devicePort
	 *            Port for device. Default is 4545
	 * @param hostPort
	 *            Port for this computer. Default is 4550
	 */
	public DataHandler(String[] inNames, String[] outNames,
			int updateFrequency, boolean debugPrint, String deviceIP,
			int devicePort, int hostPort) {
		this.inNames = inNames;
		this.outNames = outNames;
		debug = debugPrint;

		udp = new UDP(this, hostPort); // create a new datagram connection
		// udp.log(true); // <-- printout the connection activity
		udp.listen(true); // and wait for incoming message
		inNums = new int[inNames.length];
		outNums = new int[outNames.length];
		for (int i = 0; i < outNums.length; i++) {
			outNums[i] = 0;
		}
		this.deviceIP = deviceIP;
		this.devicePort = devicePort;
		lastUpdate = 0;
		timeout = -1;

		timer = new Timer(true);
		timer.scheduleAtFixedRate(new Updater(), 0, updateFrequency);
	}

	/**
	 * @param inNames
	 * @param outNames
	 * @param updateFrequency
	 * @param debug
	 */
	public DataHandler(String[] inNames, String[] outNames,
			int updateFrequency, boolean debug) {
		// default IP and port defined here
		this(inNames, outNames, updateFrequency, debug, "192.168.1.117", 4545,
				4550);
	}

	/**
	 * @param inNames
	 * @param outNames
	 * @param updateFrequency
	 */
	public DataHandler(String[] inNames, String[] outNames, int updateFrequency) {
		this(inNames, outNames, updateFrequency, false);
	}

	/**
	 * @param inNames
	 * @param outNames
	 * @param debug
	 *            If debug should be active
	 */
	public DataHandler(String[] inNames, String[] outNames, boolean debug) {
		this(inNames, outNames, 100, debug);
	}

	/**
	 * @param inNames
	 * @param outNames
	 */
	public DataHandler(String[] inNames, String[] outNames) {
		this(inNames, outNames, 100, false);
	}

	/**
	 * get a value from values received from the arduino
	 * 
	 * @param name
	 *            The name for the value requested
	 * @return the value
	 * @throws Exception
	 *             If no name exists in the outValues
	 */
	public int getValue(String name) throws Exception {
		for (int i = 0; i < inNames.length; i++) {
			if (inNames[i].equals(name)) {
				synchronized (inNums) {
					return inNums[i];
				}
			}
		}
		throw new Exception("No data found bound to that key");
	}

	/**
	 * Set a value to be sent to the arduino
	 * 
	 * @param name
	 *            The Name for the variable to be set
	 * @param value
	 *            The value to set Name to
	 * @return Success
	 */
	public boolean setValue(String name, int value) {
		for (int i = 0; i < outNames.length; i++) {
			if (outNames[i].equals(name)) {
				synchronized (outNums) {
					outNums[i] = value;
				}
				return true;
			}
		}
		return false;
	}

	private void updateOutNums() {
		ByteBuffer upStream = ByteBuffer.allocate(outNums.length * 6);
		upStream.order(ByteOrder.LITTLE_ENDIAN);
		synchronized (outNums) {
			for (byte i = 0; i < outNums.length; i++) {
				upStream.put((byte) 0).put((byte) (i + 1)).putInt(outNums[i]);
			}
		}
		udp.send(upStream.array(), deviceIP, devicePort);
		if (debug) {
			String s = "";
			for (int i = 0; i < outNames.length; i++) {
				s += outNames[i] + ":" + outNums[i] + ", ";
			}
			print(s);
		}
	}

	/**
	 * Sets a listener for device timeout, will call a.actionPerformed(null)
	 * when it has been timeout ms since last arduino response If timeout is
	 * negative, removes any timeout listener set previously
	 * 
	 * @param a
	 * @param timeout
	 */
	public void setDeviceTimeoutListener(ActionListener a, long timeout) {
		if (timeout >= 0) {
			timeoutAction = a;
			this.timeout = timeout;
		} else {
			timeout = -1;
			timeoutAction = null;
		}
	}

	/**
	 * Get the time since the device last sent an update
	 * 
	 * @return Ms since last update was received
	 */
	public long getTimeSinceLastUpdate() {
		return (System.currentTimeMillis() - lastUpdate);
	}

	/**
	 * Handles data captured DO NOT CALL METHOD FROM CLIENT. This method is
	 * called from the UDP communication library automatically
	 * 
	 * @param data
	 */
	public void receive(byte[] data) {
		lastUpdate = System.currentTimeMillis();
		int i = 0;
		ByteBuffer reader = ByteBuffer.wrap(data);
		reader.order(ByteOrder.LITTLE_ENDIAN);
		synchronized (inNums) {
			while (i < data.length) {
				if (data[i] == 0 && (data.length - i) >= 6 && data[i + 1] > 0
						&& data[i + 1] <= inNums.length) {
					inNums[data[i + 1] - 1] = reader.getInt(i + 2);
					i += 6;
				} else {
					i++;
				}
			}
		}
		if (debug) {
			String s = "";
			for (i = 0; i < inNames.length; i++) {
				s += inNames[i] + ":" + inNums[i] + ", ";
			}
			print(s);
		}
	}

	private void print(String s) {
		if (debug) {
			System.out.println(s);
		}
	}
}
