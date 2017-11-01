package upsclient;


/**
 * Trida pro predavani informaci o hre mezi tridami
 * 
 * @author Lukas Haringer
 *
 */
public class Game {
	
	String jmenoProtihrace = "";
	int idHry = 0;
	
	public Game(String jmenoProtihrace, int idHry){
		this.jmenoProtihrace = jmenoProtihrace;
		this.idHry = idHry;
	}
}
