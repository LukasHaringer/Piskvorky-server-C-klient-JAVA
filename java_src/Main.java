package upsclient;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.swing.JOptionPane;

/**
 * @author Lukas Haringer
 *
 */
public class Main {

    private int velikostPlochy = 13;

    private Control ovladani;
    private GameWindow oknoHry;
    private UdpClient komunikace;
    DatagramSocket ds;
    int portCislo;
    InetAddress address;
    byte[] sentBuffer = "                                      ".getBytes();

    int odeslanoBytu = 0;
    int prijatoBytu = 0;
    int odeslanoZprav = 0;
    int prijatoZprav = 0;

    int odehranoHer = 0;

    long start = 0;
    String nick;

    public static void logujStatistiku() {

    }

    public void pripojeni() {

        boolean nepripojeno = true;

        while (nepripojeno) {

            ovladani.nickVstup.setText("");
            ovladani.adresaVstup.setText("127.0.0.1");
            ovladani.portVstup.setText("10005");

            ovladani.oknoPripojeni();

            nick = ovladani.nickVstup.getText();

            if (nick.contains("|") || nick.length() < 3 || nick.length() > 15) {
                JOptionPane.showMessageDialog(null,
                        "Nick hrace musi byt dlouhy 3 - 15 znaku a nesmi obsahovat znak \"|\" ",
                        "Neplatny nick hrace.",
                        JOptionPane.ERROR_MESSAGE);
                ovladani.nickVstup.setText("");
                continue;
            }

            String adresa = ovladani.adresaVstup.getText();
            String port = ovladani.portVstup.getText();

            try {
                portCislo = Integer.parseInt(port);
                address = InetAddress.getByName(adresa);

            } catch (NumberFormatException e) {
                JOptionPane.showMessageDialog(null,
                        "Neplatne cislo adresy nebo portu.",
                        "Neplatne cislo",
                        JOptionPane.ERROR_MESSAGE);
                ovladani.nickVstup.setText("");
                continue;
            } catch (UnknownHostException ex) {
                Logger.getLogger(Main.class.getName()).log(Level.SEVERE, null, ex);
            }

            try {
                ds = new DatagramSocket();

                System.out.println("Prihlasuji nick: " + nick);

                ovladani.setTitle("Piskvorky - " + ovladani.nickVstup.getText());

                nepripojeno = false;

            } catch (Exception e) {
                ovladani.adresaVstup.setText("");
                ovladani.portVstup.setText("");
                JOptionPane.showMessageDialog(null,
                        "K serveru se nelze pripojit, zkontrolujte vstupni udaje.",
                        "Pripojovani selhalo.",
                        JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    public void spustKlienta() {
        ovladani = new Control(this);
        pripojeni();
        komunikace = new UdpClient(this, ds, address, portCislo);
        komunikace.setGUI(ovladani);
        ovladani.setKomunikace((IUdpClient) komunikace);
        String logZprava = "1|" + nick;
        if (ovladani.reconnect.isSelected()) {
            logZprava = "r|" + nick;
        }
        komunikace.sendMessage(logZprava);
        ovladani.setVisible(true);
        komunikace.start();
    }

    public Main() {
        start = System.currentTimeMillis();
    }

    /**
     * @param args
     */
    public static void main(String[] args) {

        try {
            PrintWriter pw = new PrintWriter(new BufferedWriter(new FileWriter(new File("log.txt"), true)));
            pw.println("\n\n NOVE SPUSTENI KLIENTA");
            pw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        Main main = new Main();
        //main.pripojeni();
        main.spustKlienta();

        System.out.println("Program dobehl");
    }

    public int getOdeslanoBytu() {
        return odeslanoBytu;
    }

    public void setOdeslanoBytu(int odeslanoBytu) {
        this.odeslanoBytu = odeslanoBytu;
    }

    public int getPrijatoBytu() {
        return prijatoBytu;
    }

    public void setPrijatoBytu(int prijatoBytu) {
        this.prijatoBytu = prijatoBytu;
    }

    public int getOdeslanoZprav() {
        return odeslanoZprav;
    }

    public void setOdeslanoZprav(int odeslanoZprav) {
        this.odeslanoZprav = odeslanoZprav;
    }

    public int getPrijatoZprav() {
        return prijatoZprav;
    }

    public void setPrijatoZprav(int prijatoZprav) {
        this.prijatoZprav = prijatoZprav;
    }

}
