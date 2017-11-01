package upsclient;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;


/**
 * Trida dedi JButton, jedna instance slouzi jako jedno policko hraci plochy
 * Zaroven obsahuje informace o svoji poloze a hodnote 
 * 
 * @author Lukas Haringer
 *
 */
public class GameButton extends JButton{
	
	private int x;
	private int y;
	
	int rozmer = 0;
	
	private int vyskaFontu = 17;
	private Font font = new Font("Arial",Font.BOLD,vyskaFontu);
	
	private UdpClient komunikace;
	// 2 je empty, 0 kolecko, 1 krizek
	int hodnota = 2;
	
	/**
	 * Nastavi vlastnosti tlacitka a zaroven nastavi parametry
	 * do lokalnich promennych
	 * 
	 * @param x vodorovna souradnice tlacitka v hraci plose
	 * @param y svisla souradnice tlacitka  v hraci plose
	 * @param rozmer sirka tlacitka na hraci plose
	 */
	public GameButton(int x, int y, int rozmer){
		this.setPreferredSize(new Dimension(rozmer, rozmer));
	
		this.setBackground(Color.WHITE);
		
		this.x = x;
		this.y = y;
		this.rozmer = rozmer;
		
		int in = -5;
		this.setMargin(new Insets(in, in+1, in, in));
		this.setFont(font);
		this.setText("");
		
		this.setEnabled(true);
		this.setVisible(true);
		//centrovani textu, tucne pismo, bile pozadi, hranate rohy
		
		spustPosluchac();
	}

	
	/**
	 * Nastavi hodnotu tlacitka, to podle toho zmeni svuj text
	 * 
	 * @param hodnota
	 */
	public void setHodnota(int hodnota){
		this.hodnota = hodnota;
		if(hodnota == 0){
			this.setText("O");
                        this.setForeground(Color.blue);
		}else{
			if(hodnota == 1){
				this.setText("X");
                                this.setForeground(Color.red);
			}
	
			if(hodnota == 2){
				this.setText("");
			}
		}
	}
	
	/**
	 * Spusti posluchac tlacitka, ten v pripade stisku tento vyhodnoti
	 * a provede potrebne akce 
	 */
	public void spustPosluchac(){
		
		this.addActionListener(new ActionListener() {
			
			@Override
			public void actionPerformed(ActionEvent arg0) {
				if(hodnota== 2){
					komunikace.sendMove(x, y);
					
				}else{
					//nic se nestane, protoze pole uz hodnotu ma
					System.out.println("Pole uz hodnotu ma.");
				}
				
				
			}
		});
		
	}
	
	public void setKomunikace(UdpClient komunikace){
		this.komunikace = komunikace;
	}
	
	
}
