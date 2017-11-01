package upsclient;

import java.awt.Container;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import javax.swing.*;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

/**
 * Trida slouzi k zobrazeni seznamu her, pripojeni do hry
 *
 * @author Lukas Haringer
 *
 */
public class Control extends JFrame implements ListSelectionListener {

    private IUdpClient komunikace;

    /* Promenne pro dialog nacteni vstupnich udaju*/
    JTextField adresaVstup = new JTextField();

    Main main;

    JTextField portVstup = new JTextField();
    JTextField nickVstup = new JTextField();
    JLabel adresaLabel = new JLabel("Adresa serveru");
    JLabel portLabel = new JLabel("Cislo portu");
    JLabel nickLabel = new JLabel("Nick - max. 15 znaku");
    JCheckBox reconnect = new JCheckBox("Pripojit po padu hry");

    private JList<String> listHer;
    private DefaultListModel<String> listModel;

    private JButton newGameButton;
    private JButton gameConnectButton;

    private JPanel tlacitkaPanel;
    private boolean zalozenaHra = false;

    /**
     * Konstruktor nastavujici parametry okna
     *
     * @param main
     */
    public Control(Main main) {

        this.main = main;

        this.setTitle("Piskvorky");
        this.setLocationRelativeTo(null);
        this.setSize(300, 430);
        this.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        this.setResizable(false);
        this.add(getHlavniPanel());

        spustPosluchace();

        this.setVisible(false);
        this.repaint();
    }

    /**
     * Vytvori hlavni panely aplikace
     *
     * @return panel obsahujici veskery obsah okna
     */
    public Container getHlavniPanel() {
        JPanel hlavniPanel = new JPanel();

        //hlavniPanel.setLayout(new FlowLayout());
        hlavniPanel.add(vytvorList());

        hlavniPanel.add(vytvorTlacitkaPanel());

        hlavniPanel.setVisible(true);

        return hlavniPanel;
    }

    /**
     * Metoda vytvori skrolovaci seznam
     *
     * @return skrolovaci seznam her
     */
    public JScrollPane vytvorList() {
        listModel = new DefaultListModel();
        //listModel.addElement("Hra: 0  DUMMY");

        listHer = new JList(listModel);
        listHer.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        listHer.setSelectedIndex(0);
        listHer.addListSelectionListener(this);
        listHer.setVisibleRowCount(7);
        listHer.setVisible(true);

        JScrollPane listScrollPane = new JScrollPane(listHer);
        listScrollPane.setPreferredSize(new Dimension(250, 300));
        listScrollPane.setVisible(true);
        listScrollPane.setName("hry");

        return listScrollPane;
    }

    /**
     * Vytvori panel s tlacitky
     *
     * @return panel s tlacitky
     */
    public JPanel vytvorTlacitkaPanel() {
        newGameButton = new JButton("Zaloz hru");
        newGameButton.setPreferredSize(new Dimension(140, 30));
        gameConnectButton = new JButton("Pripoj ke hre");
        gameConnectButton.setPreferredSize(new Dimension(140, 30));

        tlacitkaPanel = new JPanel();
        tlacitkaPanel.add(newGameButton);
        tlacitkaPanel.add(gameConnectButton);
        tlacitkaPanel.setPreferredSize(new Dimension(140, 200));
        tlacitkaPanel.setVisible(true);

        return tlacitkaPanel;
    }

    /**
     * Metoda spusti posluchace tlacitek
     */
    public void spustPosluchace() {
        newGameButton.addActionListener(new ActionListener() {

            @Override
            public void actionPerformed(ActionEvent arg0) {

                // posli komunikaci at zalozi hru
                if (zalozenaHra == false) {
                    gameConnectButton.setEnabled(false);
                    newGameButton.setText("Zrus zalozeni");
                    zalozenaHra = true;
                    komunikace.CreateGame();
                } else {
                    gameConnectButton.setEnabled(true);
                    newGameButton.setText("Zaloz hru");
                    zalozenaHra = false;
                    komunikace.cancelGame();
                }

            }
        });

        gameConnectButton.addActionListener(new ActionListener() {

            @Override
            public void actionPerformed(ActionEvent e) {
                int polozka = listHer.getSelectedIndex();
                if (polozka >= 0) {
                    newGameButton.setEnabled(false);
                    gameConnectButton.setEnabled(false);
                    komunikace.connectToGame(polozka);
                }

            }
        });

        this.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                komunikace.disconnect("f");
                zalogujSkonci();
            }
        });

    }

    /**
     * Pri zavreni klienta zaloguje statistiky
     */
    public void zalogujSkonci() {
        PrintWriter pw;
        try {
            pw = new PrintWriter(new BufferedWriter(new FileWriter(new File("logClient.txt"), true)));
            pw.println("Odeslano bytu: " + main.odeslanoZprav);
            pw.println("Prijato bytu: " + main.prijatoBytu);
            pw.println("Odeslano zprav: " + main.odeslanoZprav);
            pw.println("Prijato zprav: " + main.prijatoZprav);
            pw.println("Odehrano her: " + main.odehranoHer);
            pw.println("Doba behu: " + (System.currentTimeMillis() - main.start) / 1000 + " sekund.");
            pw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        System.exit(0);
    }

    /**
     * Ze seznamu her naplni strukturu modelu seznamu v grafice
     *
     * @param seznam arraylist her
     */
    public void nastavSeznamHer(ArrayList<Game> seznam) {

        listModel.clear();
        for (Game hra : seznam) {
            listModel.addElement("Hra: " + hra.idHry + " zalozil hrac: " + hra.jmenoProtihrace);
        }

    }

    /**
     * Nacte prihlasovaci udaje
     */
    public void oknoPripojeni() {

        JComponent[] prvky = new JComponent[7];
        prvky[0] = adresaLabel;
        prvky[1] = adresaVstup;
        prvky[2] = portLabel;
        prvky[3] = portVstup;
        prvky[4] = nickLabel;
        prvky[5] = nickVstup;
        prvky[6] = reconnect;

        if (JOptionPane.showConfirmDialog(null, prvky, "Pripojeni k serveru", JOptionPane.OK_CANCEL_OPTION) == 2) {

            System.exit(0);
        }
    }

    /**
     * Zablokuje vsechna tlacitka
     */
    public void deaktivujOvladani() {
        gameConnectButton.setEnabled(false);
        gameConnectButton.setEnabled(false);
    }

    /**
     * Povoli vsechna tlacitka a vrati jim vychozi hodnotu
     */
    public void resetOvladani() {
        this.setEnabled(true);
        gameConnectButton.setEnabled(true);
        gameConnectButton.setText("Pripoj ke hre");

        newGameButton.setEnabled(true);
        newGameButton.setText("Zaloz hru");

        zalozenaHra = false;

    }

    @Override
    public void valueChanged(ListSelectionEvent arg0) {
        // TODO Auto-generated method stub

    }

    public IUdpClient getKomunikace() {
        return komunikace;
    }

    public void setKomunikace(IUdpClient komunikace) {
        this.komunikace = komunikace;
    }

    public JButton getZalozHruBT() {
        return newGameButton;
    }

    public void setZalozHruBT(JButton zalozHruBT) {
        this.newGameButton = zalozHruBT;
    }

    public JButton getPripojKeHreBT() {
        return gameConnectButton;
    }

    public void setPripojKeHreBT(JButton pripojKeHreBT) {
        this.gameConnectButton = pripojKeHreBT;
    }
}
