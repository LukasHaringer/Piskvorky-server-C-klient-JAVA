package upsclient;


/**
 * Rozhrani pro komunikace GUI s tridou Komunikace 
 * 
 * @author Lukas Haringer
 *
 */
public interface IUdpClient {

	/**
	 * Metoda zajisti pripojeni ke hre vybrane v seznamu her
	 * 
	 * @param indexHry index hry v seznamu her
	 */
	public void connectToGame(int indexHry);
	
	/**
	 * Posle prikaz k zalozeni hry
	 */
	public void CreateGame();
		
	/**
	 * 	Zrusi zalozenou hru pokud se zatim nepripojil dalsi hrac
	 */
	public void cancelGame();
	
	/**
	 *  Posle prikaz ke vzdani hry, to nastane pokud hrac
	 *  klikne na tlacitlo "Vzdej hru" nebo zavre okno se hrou
	 */
	public void giveUp();
	
	/**
	 * Posle prikaz k odeslani tahu na souradnicich v parametrech
	 * 
	 * @param x vodorovna souradnice policka
	 * @param y svisla souradnice policka
	 */
	public void sendMove(int x, int y);
        
        public void disconnect(String zprava);

}
