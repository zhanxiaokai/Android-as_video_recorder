/**
 * 
 */
package com.changba.songstudio.util;

import android.util.Base64;

import java.security.Key;

import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;
import javax.crypto.spec.IvParameterSpec;

public class DesEncode {

	public static final String ALGORITHM_DES = "DES/CBC/PKCS5Padding";

	/**
	 * DES算法，加密
	 * 
	 * @param data
	 *            待加密字符串
	 * @param key
	 *            加密私钥，长度不能够小于8位
	 * @return 加密后的Base64编码
	 *             异常
	 */
	public static String encode(String key, String data) throws Exception {
		return encode(key, data.getBytes());
	}
	
	public static String encodeUrl(String key, String data) throws Exception {
		return encodeUrl(key, data.getBytes());
	}

	private static String encode(String key, byte[] data) throws Exception {

		DESKeySpec dks = new DESKeySpec(key.getBytes());

		SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");
		// key的长度不能够小于8位字节
		Key secretKey = keyFactory.generateSecret(dks);
		Cipher cipher = Cipher.getInstance(ALGORITHM_DES);
		cipher.init(Cipher.ENCRYPT_MODE, secretKey, new IvParameterSpec("12345678".getBytes()));

		byte[] bytes = cipher.doFinal(data);
		try{
			return Base64.encodeToString(bytes, Base64.URL_SAFE);
		}catch(NoClassDefFoundError e){//低版本的android SDK没有Base64 class
			String encodeStr = com.changba.songstudio.util.Base64.encode(bytes);
			encodeStr = encodeStr.replaceAll("\\+", "-").replaceAll("/", "_").replaceAll("=", "");
			return encodeStr;
		}
	}
	/**
	 * * encodeUrl ,url 中涉及到\n的将会被服务器截断，所以要自定义encode
	 * @param key
	 * @param data
	 * @return
	 * @throws Exception
	 */
	private static String encodeUrl(String key, byte[] data) throws Exception {

		DESKeySpec dks = new DESKeySpec(key.getBytes());

		SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");
		// key的长度不能够小于8位字节
		Key secretKey = keyFactory.generateSecret(dks);
		Cipher cipher = Cipher.getInstance(ALGORITHM_DES);
		cipher.init(Cipher.ENCRYPT_MODE, secretKey, new IvParameterSpec("12345678".getBytes()));

		byte[] bytes = cipher.doFinal(data);
		String encodeStr = com.changba.songstudio.util.Base64.encode(bytes);
		encodeStr = encodeStr.replaceAll("\\+", "-").replaceAll("/", "_").replaceAll("=", "").replaceAll("\r", "").replaceAll("\n", "").replaceAll(" ", "");
		return encodeStr;
	}
	
	
	/**
	 * 获取编码后的值
	 * 
	 * @param key
	 * @param data
	 * @return
	 * @throws Exception
	 */
	public static String decode(String key, String data) throws Exception {
		if(null != data && data.trim().length() <= 0){
			return "";
		}
		byte[] datas;
		try{
			datas = decode(key, Base64.decode(data, Base64.URL_SAFE));
		}catch(Exception e){//低版本的android SDK没有Base64 class
			data= data.replaceAll("-", "+").replaceAll("_", "/");
			datas = decode(key, 
					com.changba.songstudio.util.Base64.decode(data));
		}
		return new String(datas);
	}
	
	/**
	 * 获取编码后的值  ,url 中涉及到\n的将会被服务器截断，所以要自定义encode
	 * 
	 * @param key
	 * @param data
	 * @return
	 * @throws Exception
	 */
	public static String decodeUrl(String key, String data) throws Exception {
		byte[] datas;
		data= data.replaceAll("-", "+").replaceAll("_", "/");
		datas = decode(key, 
				com.changba.songstudio.util.Base64.decode(data));
		return new String(datas);
	}

	public static byte[] decode(String key, byte[] data) throws Exception {

		DESKeySpec dks = new DESKeySpec(key.getBytes());
		SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");
		// key的长度不能够小于8位字节
		Key secretKey = keyFactory.generateSecret(dks);
		Cipher cipher = Cipher.getInstance(ALGORITHM_DES);
		cipher.init(Cipher.DECRYPT_MODE, secretKey, new IvParameterSpec("12345678".getBytes()));
		return cipher.doFinal(data);

	}

}