package upsclient;

import java.awt.Container;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.WindowConstants;

/**
 * Okno ve kterem probiha hra
 *
 * @author Lukas Haringer
 *
 */
public class GameWindow extends JFrame {

    private IUdpClient komunikace;

    private int pocetPolicek = 10;
    private int odsazeni = -1;

    JPanel hlavniPanel;
    JPanel ovladani;
    JPanel hraciPlocha;

    JLabel infoLabel;
    JLabel naTahuLabel;

    JButton vzdejHruBT;

    private GameButton[][] tlacitkaPole;

    /**
     * Konstruktor nastavi parametry a velikost okna
     *
     * @param pocetPolicek pocet policek hraci plochy
     * @param tlacitkaPole pole obsahujici objekty s tlacitky
     */
    public GameWindow(int pocetPolicek, GameButton[][] tlacitkaPole) {

        this.tlacitkaPole = tlacitkaPole;
        this.pocetPolicek = pocetPolicek;

        this.setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
        this.setTitle("Piskvorky");
        this.setLocationRelativeTo(null);
        this.setSize(500, 650);
        this.setResizable(false);
        this.add(getHlavniPanel());
        this.setVisible(true);
        this.repaint();
    }

    /**
     * Zajisti vytvoreni hlavniho panelu a zavola metody pro vytvoreni
     * dcerinnych panelu
     *
     * @return panel obsahujici veskery obsah okna
     */
    public Container getHlavniPanel() {
        hlavniPanel = new JPanel();

        hlavniPanel.add(vytvorHraciPlochu());

        hlavniPanel.add(vytvorOvladani());

        hlavniPanel.setVisible(true);
        return hlavniPanel;

    }

    /**
     * Vytvori ovladaci tlacitka
     *
     * @return panel s ovladacimi tlacitky
     */
    public JPanel vytvorOvladani() {
        ovladani = new JPanel();
        ovladani.setPreferredSize(new Dimension(230, 550));

        infoLabel = new JLabel("                       ");
        infoLabel.setPreferredSize(new Dimension(500, 25));

        naTahuLabel = new JLabel("", JLabel.CENTER);
        naTahuLabel.setPreferredSize(new Dimension(220, 50));
        naTahuLabel.setFont(new Font("Serif", Font.PLAIN, 30));
        
        vzdejHruBT = new JButton("Vzdej hru");
        vzdejHruBT.setPreferredSize(new Dimension(800, 60));

        //ovladani.add(infoLabel);
        //ovladani.add(jmenoProtihraceLabel);
        ovladani.add(naTahuLabel);
        ovladani.add(vzdejHruBT);
        spustPosluchace();

        return ovladani;
    }

    /**
     * Spusti posluchace tlacitek
     */
    public void spustPosluchace() {

        vzdejHruBT.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
                vzdejHru(0);
            }
        });

        this.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {

                vzdejHru(1);

            }
        });

    }

    /**
     * Zavola metodu tridy komunikace pro vzdani hry.
     *
     * @param akce druh konce hry
     */
    public void vzdejHru(int akce) {
        this.setEnabled(false);

        if (akce == 0) {

            if (JOptionPane.showConfirmDialog(null, "Skutecne chcete hru vzdat?", "Ukoncit", JOptionPane.OK_CANCEL_OPTION) == 2) {
                this.setEnabled(true);
                System.out.println("Hra pokracuje");

            } else {
                komunikace.giveUp();

            }

        }
        if (akce == 1) {
            if (JOptionPane.showConfirmDialog(null, "Zavrenim okna prohrajete!", "Ukoncit", JOptionPane.OK_CANCEL_OPTION) == 2) {
                this.setEnabled(true);
                System.out.println("Hra pokracuje");

            } else {
                komunikace.giveUp();

            }

        }

    }

    /**
     * Vytvori panel hraci plochy a naplni ho tlacitky ->policky
     *
     * @return panel s tlacitky
     */
    public JPanel vytvorHraciPlochu() {
        hraciPlocha = new JPanel();
        hraciPlocha.setLayout(null);
        int velikost = pocetPolicek * (odsazeni + tlacitkaPole[0][0].rozmer) + 2;
        hraciPlocha.setPreferredSize(new Dimension(velikost, velikost));
        pridejTlacitka(hraciPlocha);

        hraciPlocha.setVisible(true);
        return hraciPlocha;
    }

    /**
     * Prida tlacitka do panelu hraci plocha
     *
     * @param hraciPlocha panel hraci plochy pro pridani tlacitek
     */
    public void pridejTlacitka(JPanel hraciPlocha) {

        GameButton pb = null;

        for (int i = 0; i < pocetPolicek; i++) {
            for (int j = 0; j < pocetPolicek; j++) {

                pb = tlacitkaPole[i][j];
                int sirkaPolicka = pb.rozmer;
                pb.setBounds(j * sirkaPolicka + j * odsazeni,
                        i * sirkaPolicka + i * odsazeni, sirkaPolicka, sirkaPolicka);
                hraciPlocha.add(pb);
            }
        }

    }

    public IUdpClient getKomunikace() {
        return komunikace;
    }

    public void setKomunikace(IUdpClient komunikace) {
        this.komunikace = komunikace;
    }

}
