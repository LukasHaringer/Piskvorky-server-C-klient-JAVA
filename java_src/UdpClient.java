package upsclient;

import java.awt.Color;
import java.util.*;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketTimeoutException;
import java.util.ArrayList;
import java.util.Scanner;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JOptionPane;

/**
 * Trida zajistuje komunikaci se serverem a veskerou logiku na strane serveru
 *
 * @author Lukas Haringer
 *
 */
public class UdpClient extends Thread implements IUdpClient {

    private Control ovladani;
    private GameWindow oknoHry;
    File f = new File("log.txt");
    PrintWriter pw;

    private ArrayList<Game> seznamHer = new ArrayList<Game>();
    Scanner sc = new Scanner(System.in);
    public static final int VELIKOST_PLOCHY = 16;
    public static final int sirkaPolicka = 30;
    public static int packetNumberRecv = 1;
    public static int packetNumberSend = 1;
    long now = System.currentTimeMillis();
    long checked = System.currentTimeMillis();

    /*
	private BufferedReader input;
	private PrintWriter output;
	private Socket socket;
     */
    DatagramSocket socket;
    int portCislo;
    InetAddress address;
    byte[] sentBuffer = "                                      ".getBytes();
    byte[] recvBuffer = getRecvBuff();
    String kOdeslani;
    boolean odeslat = false;

    private Main main;

    private Game aktualniHra;
    private String jmenoProtihrace = "Protihrac";
    private int cisloPartie = 0;
    private int symbol = -1;
    private int symbolSoupere = -1;
    private boolean naRade = false;
    private boolean serverEnd = false;
    private GameButton[][] tlacitkaPole = new GameButton[VELIKOST_PLOCHY][VELIKOST_PLOCHY];

    // 0 - pripojen k serveru, 1 - zalozena hra, 2 - ve hre
    // pripojeni
    private int stav = 0;

    public byte[] getRecvBuff() {
        String s = "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           ";
        return s.getBytes();
    }

    /**
     * Po pripojeni do hry tato metoda vytvori nove okno hry a preda mu potrebna
     * data.
     */
    public void createGameWindow() {
        oknoHry = new GameWindow(VELIKOST_PLOCHY, tlacitkaPole);
        oknoHry.setKomunikace(this);

        for (int i = 0; i < VELIKOST_PLOCHY; i++) {
            for (int j = 0; j < VELIKOST_PLOCHY; j++) {
                tlacitkaPole[i][j].hodnota = 2;
                tlacitkaPole[i][j].setText("");
                tlacitkaPole[i][j].setKomunikace(this);
            }
        }

    }

    public void cekani() {
        JOptionPane.showMessageDialog(null,
                "Protihrac se odpojil. Pockej zda se vrati nebo vzdej hru.",
                "Upozorneni",
                JOptionPane.ERROR_MESSAGE);
    }

    /* (non-Javadoc)
	 * @see IKomunikace#pripojKeHre(int)
     */
    public void connectToGame(int indexHry) {
        if (indexHry <= seznamHer.size()) {
            jmenoProtihrace = seznamHer.get(indexHry).jmenoProtihrace;
            aktualniHra = seznamHer.get(indexHry);
            odeslat = true;
            kOdeslani = "c|" + aktualniHra.idHry;
        }

    }

    public boolean checkMessage(String s) {
        String pattern = "^[a-zA-Z0-9| ]*$";
        if (s.matches(pattern)) {
            return false;
        }
        return true;
    }

    /* (non-Javadoc)
	 * @see IKomunikace#zalozHru()
     */
    public void CreateGame() {
        odeslat = true;
        kOdeslani = "b";
        stav = 1;
    }

    /* (non-Javadoc)
	 * @see IKomunikace#zrusZalozenou()
     */
    public void cancelGame() {
        odeslat = true;
        kOdeslani = "d";
    }

    /* (non-Javadoc)
	 * @see IKomunikace#posliTah(int, int)
     */
    public void sendMove(int x, int y) {
        if (naRade == true) {
            odeslat = true;
            kOdeslani = "g|" + x + "|" + y;
            nastavNaRade(false);
        } else {
            System.out.println("Nejsi na rade, nemuzes tahnout.");
        }
    }

    /* (non-Javadoc)
	 * @see IKomunikace#vzdejHru()
     */
    public void giveUp() {
        System.out.println("Vzdal jsi hru.");
        stav = 0;
        closeGameWindow();
        //oknoHry.dispatchEvent(new WindowEvent(oknoHry, WindowEvent.WINDOW_CLOSING));
        odeslat = true;
        kOdeslani = "e|";
    }

    /**
     * Vypise do konzole hraci plochu
     */
    public void printBoard() {
        for (int i = 0; i < VELIKOST_PLOCHY; i++) {
            for (int j = 0; j < VELIKOST_PLOCHY; j++) {

                GameButton pom = tlacitkaPole[i][j];
                if (pom.hodnota == 2) {
                    System.out.print("- ");
                } else if (pom.hodnota == 1) {
                    System.out.print("X ");
                } else {
                    System.out.print("O ");
                }

            }
            System.out.println();
        }
    }

    /**
     * Metoda odesle zpravu a zavola metodu pro jeji zalogovani
     *
     * @param zprava text k odeslani
     */
    public void sendMessage(String zprava) {
        int pocitadlo = 0;

        zprava = packetNumberSend + "|" + zprava;
        main.odeslanoBytu += zprava.length();
        main.odeslanoZprav++;
        System.out.println(zprava);
        loguj("posilam: " + zprava);

        sentBuffer = zprava.getBytes();
        DatagramPacket send = new DatagramPacket(sentBuffer, sentBuffer.length, address, portCislo);
        try {
            socket.send(send);
        } catch (IOException ex) {
            System.out.println("Chyba pri odeslani datagramu");
        }
        System.out.println("Posilam zpravu: " + zprava);

        long nowS = System.currentTimeMillis();
        long checkedS = System.currentTimeMillis();
        while (pocitadlo < 10) {
            try {
                socket.setSoTimeout(1000);
                DatagramPacket recv = new DatagramPacket(recvBuffer, recvBuffer.length);
                socket.receive(recv);

                String zpravaPrijata;
                if ((zpravaPrijata = new String(recvBuffer)).equals("") || checkMessage(zpravaPrijata)) {
                    continue;
                }
                checked = System.currentTimeMillis();
                String[] pole = zpravaPrijata.split("\\|");

                if (convertNumber(pole[0]) == packetNumberRecv - 1 && packetNumberRecv - 1 != 0) {
                    sendRenameAck();
                    continue;
                }
                if (pole[1].equals("p")) {
                    int cisloOvereni = convertNumber(pole[2]);
                    if (cisloOvereni == packetNumberSend) {
                        System.out.println("Doruceno");
                        break;
                    }
                }

                recvBuffer = getRecvBuff();
            } catch (SocketTimeoutException e) {
                pocitadlo++;
                if (pocitadlo == 10) {
                    JOptionPane.showMessageDialog(null,
                            "Server je nedostupny, klient bude ukoncen.",
                            "Nedostupny server",
                            JOptionPane.ERROR_MESSAGE);
                    System.exit(1);
                }
                try {
                    nowS = System.currentTimeMillis();
                    if ((nowS - checkedS) > 2500) {
                        socket.send(send);
                        System.out.println("posilam");
                        checkedS = System.currentTimeMillis();
                    }
                } catch (IOException ex) {
                    Logger.getLogger(UdpClient.class.getName()).log(Level.SEVERE, null, ex);
                }
                recvBuffer = getRecvBuff();
            } catch (IOException ex) {
                Logger.getLogger(UdpClient.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        packetNumberSend++;
    }

    public void disconnect(String zprava) {
        zprava = packetNumberSend + "|" + zprava;
        main.odeslanoBytu += zprava.length();
        main.odeslanoZprav++;
        System.out.println(zprava);
        loguj("posilam: " + zprava);

        sentBuffer = zprava.getBytes();
        DatagramPacket send = new DatagramPacket(sentBuffer, sentBuffer.length, address, portCislo);
        try {
            socket.send(send);
        } catch (IOException ex) {
            System.out.println("Chyba pri odeslani datagramu");
        }
        System.out.println("Posilam zpravu: " + zprava);
    }

    public void sendAck() {

        String zprava = "0|p|" + packetNumberRecv + "|";
        main.odeslanoBytu += zprava.length();
        main.odeslanoZprav++;

        loguj("posilam: " + zprava);

        sentBuffer = zprava.getBytes();
        DatagramPacket send = new DatagramPacket(sentBuffer, sentBuffer.length, address, portCislo);
        try {
            socket.send(send);
        } catch (IOException ex) {
            System.out.println("Chyba pri odeslani datagramu");
        }
        System.out.println("Posilam zpravu: " + zprava);
    }

    public void sendRenameAck() {

        String zprava = "0|p|" + (packetNumberRecv - 1) + "|";
        main.odeslanoBytu += zprava.length();
        main.odeslanoZprav++;

        loguj("posilam: " + zprava);

        sentBuffer = zprava.getBytes();
        DatagramPacket send = new DatagramPacket(sentBuffer, sentBuffer.length, address, portCislo);
        try {
            socket.send(send);
        } catch (IOException ex) {
            System.out.println("Chyba pri odeslani datagramu");
        }
        System.out.println("Posilam zpravu: " + zprava);
    }

    /**
     * Metoda prevede cislo z textove na numerickou podobu
     *
     * @param retCislo cislo jako retezec
     * @return prevedene cislo
     */
    public int convertNumber(String retCislo) {
        int cislo = 0;
        try {
            cislo = Integer.parseInt(retCislo);
        } catch (Exception e) {
            System.out.println("Cislo se nepodarilo prevest: " + retCislo);
        }
        return cislo;
    }

    /**
     * @param zprava zprava obsahujici pocet her a jejich seznam
     */
    public void newGameList(String zprava) {
        //System.out.println("Seznam her: " + zprava);
        String[] pole = zprava.split("\\|");
        int pocetHer = convertNumber(pole[2]);
        seznamHer.clear();
        for (int i = 0; i < pocetHer; i++) {
            seznamHer.add(new Game(pole[i * 2 + 4], convertNumber(pole[i * 2 + 3])));
        }
        ovladani.nastavSeznamHer(seznamHer);
        ovladani.repaint();
    }

    /**
     * Vyhodnoti zpravu od serveru o zalozeni partie
     *
     * @param zprava
     */
    public void gameCreated(String zprava) {
        String[] pole = zprava.split("\\|");
        if (pole[2].charAt(0) == '1') {
            System.out.println("Hra uspesne zalozena");
            stav = 1;
        } else {

            System.out.println("Partii se nepodarilo zalozit");
            stav = 0;
        }
    }

    /**
     * Vyhodnoti zpravu od serveru o pripojeni a informuje o nem hrace.
     *
     * @param zprava
     */
    public void connectionSucces(String zprava) {
        String[] pole = zprava.split("\\|");
        if (pole[2].charAt(0) == '1') {
            stav = 2;
            System.out.println("Uspesne pripojeni do hry!");
            System.out.println("Hra zacala, hodne stesti");
            if (zprava.length() > 5) {
                String jmenoProtihrace = pole[3];
                jmenoProtihrace = jmenoProtihrace.replace("|", "");
                System.out.println("Jmeno protihrace: " + jmenoProtihrace);
                this.jmenoProtihrace = jmenoProtihrace;
            }
            ovladani.deaktivujOvladani();
            ovladani.setVisible(false);
        } else {

            System.out.println("K partii se nepodarilo pripojit");
            stav = 0;
            ovladani.resetOvladani();
        }
    }

    /**
     * Vyhodnoti zpravu od serveru o zrusene hre.
     *
     * @param zprava
     */
    public void gameCanceled(String zprava) {

        if (stav == 1) {
            //nic se nestane, protoze hrac je zakladajici
        } else {
            JOptionPane.showMessageDialog(null,
                    "Zakladajici hru zrusil jeste pred jejim spustenim.",
                    "Hra zrusena.",
                    JOptionPane.ERROR_MESSAGE);
            System.out.println("Hra zrusena zakladajicim hracem jeste pred zacatkem");
        }
        closeGameWindow();
    }

    /**
     * Zavre okno hry, to nastane bud po jejim skonceni, nebo po necekanem
     * ukonceni hry
     */
    public void closeGameWindow() {
        stav = 0;
        ovladani.resetOvladani();
        ovladani.setVisible(true);
        if (oknoHry != null) {
            oknoHry.dispose();
        }
    }

    /**
     * Zkontroluje zda je tah protihrace korektni a zanese ho do pole s daty.
     *
     * @param zprava
     */
    public void processMove(String zprava) {
        String[] pole = zprava.split("\\|");
        if (pole[2].charAt(0) == '1') {
            int x = -1;
            int y = -1;

            try {
                x = Integer.parseInt(pole[3]);
                y = Integer.parseInt(pole[4]);

            } catch (Exception e) {
                System.out.println("Neplatna pozice tahu " + zprava);
                x = 0;
                y = 0;
            }
            System.out.println("Platny tah x = " + x + "   y = " + y);

            tlacitkaPole[y][x].setHodnota(symbol);
            oknoHry.repaint();

        } else {
            System.out.println("Neplatny tah " + zprava);
            nastavNaRade(true);
        }
    }

    /**
     * Podle parametru nastavi promennou naRade a upravi popisky v oknu hry.
     *
     * @param isNaRade zda je hrac na rade
     */
    public void nastavNaRade(boolean isNaRade) {
        naRade = isNaRade;
        String s = "";
        if (isNaRade) {
            s = "Jsi na tahu";
            System.out.println(s);
            oknoHry.naTahuLabel.setText(s);
            oknoHry.naTahuLabel.setForeground(Color.green);

        } else {
            s = "Tahne souper";
            System.out.println(s);
            oknoHry.naTahuLabel.setText(s);
            oknoHry.naTahuLabel.setForeground(Color.red);
            // zablokovani hraci plochy
        }
    }

    /**
     *
     * Ze zpravy na serveru zjisti zda je hrac na rade a zda ma krizky nebo
     * kolecka
     *
     * @param zprava
     */
    public void naRade(String zprava) {

        createGameWindow();
        //zmizeni prihlasovani
        ovladani.setEnabled(false);
        ovladani.setVisible(false);
        String[] pole = zprava.split("\\|");
        if (pole[2].charAt(0) == '0') {

            // hrac ma kolecka
            symbol = 0;
            symbolSoupere = 1;
            nastavNaRade(false);

        } else {
            // hrac ma krizky a je na rade
            symbol = 1;
            symbolSoupere = 0;
            nastavNaRade(true);

        }
        /*nastaveni titulku okna hry*/
        if (symbol == 0) {
            oknoHry.setTitle("Piskvorky - mate kolecka \"O\"");

        } else {
            oknoHry.setTitle("Piskvorky - mate krizky \"X\"");

        }
    }

    public void reconnect(String zprava) {

        createGameWindow();
        //zmizeni prihlasovani
        ovladani.setEnabled(false);
        ovladani.setVisible(false);
        stav = 2;
        String[] pole = zprava.split("\\|");
        if (pole[3].charAt(0) == '1') {
            symbol = 0;
            symbolSoupere = 1;
        } else {
            symbol = 1;
            symbolSoupere = 0;
        }
        if (pole[2].charAt(0) == '1') {
            nastavNaRade(false);
        } else {
            nastavNaRade(true);
        }

        int index = 4;
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                if (convertNumber(pole[index]) == 0) {
                    tlacitkaPole[i][j].setHodnota(1);
                } else if (convertNumber(pole[index]) == 1) {
                    tlacitkaPole[i][j].setHodnota(0);
                } else {
                    tlacitkaPole[i][j].setHodnota(2);
                }
                index++;
            }
        }
    }

    /**
     * Zaloguje zpravu v parametru
     *
     * @param zprava text ktery ma byt zalogovan
     */
    public void loguj(String zprava) {
        try {
            PrintWriter pw = new PrintWriter(new BufferedWriter(new FileWriter(new File("log.txt"), true)));
            pw.println(zprava);
            pw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Ze zpravy o konci hry precte jaky je typ konce a provede nasledne akce.
     *
     * @param zprava text zpravy
     */
    public void gameOver(String zprava) {
        String[] pole = zprava.split("\\|");
        loguj("prijato: " + zprava);

        char konec = pole[2].charAt(0);

        if (oknoHry != null) {
            oknoHry.setEnabled(false);
        }

        String text = "";
        boolean ukazText = false;
        switch (konec) {
            case '0':
                if (stav > 0) {
                    text = "Hra byla zrusena zakladajicim jeste pred jejim spustenim";
                    ukazText = true;
                    System.out.println(text);
                }
                break;

            case '1':
                text = "Souper hru vzdal. Gratuluji k vyhre!";
                ukazText = true;
                System.out.println(text);
                break;

            case '2':

                System.out.println("Vzdal jsi hru a tim padem prohral.");
                ukazText = false;
                break;

            case '3':
                text = "Souper vyhral, preji vice stesti u pristi hry.";
                ukazText = true;
                System.out.println(text);
                break;

            case '4':
                text = "Vyhral jste, gratuluji!";
                ukazText = true;
                System.out.println(text);
                break;

            case '5':
                text = "Remiza";
                ukazText = true;
                System.out.println(text);
                break;

            case '6':
                text = "Vyhral jsi, souper neodehral v casovem limitu";
                ukazText = true;
                System.out.println(text);
                break;

            case '7':
                text = "Prohral jsi, vyprsel casovy limit pro tah";
                ukazText = true;
                System.out.println(text);
                break;

            case '8':
                text = "Prohral jsi, vyprsel casovy limit pro tah";
                ukazText = true;
                System.out.println(text);
                break;
        }

        if (ukazText == true) {
            JOptionPane.showMessageDialog(null, text, "Konec hry",
                    JOptionPane.INFORMATION_MESSAGE);

        }
        closeGameWindow();
        ovladani.resetOvladani();
        System.out.println("Konec hry uplne.");
    }

    /**
     * Zmeni hodnotu promenne serverKonci na true.
     */
    public void serverQuit() {
        serverEnd = true;
    }

    /**
     * Vyhodnoti tah soupere
     *
     * @param zprava text obsahujici informace o tahu soupere
     */
    public void tahSoupere(String zprava) {
        String[] pole = zprava.split("\\|");
        int x = -1;
        int y = -1;
        try {
            x = Integer.parseInt(pole[2]);
            y = Integer.parseInt(pole[3]);

        } catch (Exception e) {
            System.out.println("Neplatna pozice souperova tahu");
            x = 0;
            y = 0;
        }

        tlacitkaPole[y][x].setHodnota(symbolSoupere);
        nastavNaRade(true);
        System.out.println("Souper tahnul tah x = " + x + "   y = " + y);

    }

    /**
     * Metoda z prvniho znaky zpravy zjisti o jaky typ zpravy se jedna a
     * nasledne ji deleguje prislusnym metodam.
     *
     * @param zprava
     */
    public void processMessage(String zprava) {

        System.out.println("Prijata zprava: " + zprava);
        String[] pole = zprava.split("\\|");
        char znak = pole[1].charAt(0);

        switch (znak) {
            case 'a':
                newGameList(zprava);
                break;

            case 'b':
                gameCreated(zprava);
                break;

            case 'c':
                connectionSucces(zprava);
                break;

            case 'd':
                gameCanceled(zprava);
                break;

            case 'e':
                System.out.println("Chybna zprava: " + zprava);
                break;

            case 'g':
                processMove(zprava);
                printBoard();
                break;

            case 'h':
                reconnect(zprava);
                break;
            case 'i':
                naRade(zprava);
                break;
            case 'k':
                gameOver(zprava);
                break;

            case 'l':
                tahSoupere(zprava);
                printBoard();
                break;
            case 'v':
                break;
            case 'q':
                serverQuit();
                break;
            case 'z':
                cekani();
                break;
            default:
                packetNumberRecv--;
                break;
        }
    }

    /**
     * Metoda v cyklu cte data ze socketu a zajistuje jejich zpracovani
     */
    public void receive() {
        boolean beh = true;
        String zprava = "";
        now = System.currentTimeMillis();
        checked = System.currentTimeMillis();

        while (beh) {
            try {

                try {
                    now = System.currentTimeMillis();
                    socket.setSoTimeout(10);
                    //	socket.set

                    if ((now - checked) > 10000) {
                        checked = System.currentTimeMillis();
                        sendMessage("v|");
                    }

                    if (odeslat) {
                        sendMessage(kOdeslani);
                        odeslat = false;
                    }

                    DatagramPacket recv = new DatagramPacket(recvBuffer, recvBuffer.length);
                    socket.receive(recv);
                    while ((zprava = new String(recvBuffer)) != null) {

                        if (zprava.equals("") || checkMessage(zprava)) {
                            recvBuffer = getRecvBuff();
                            recv = new DatagramPacket(recvBuffer, recvBuffer.length);
                            socket.receive(recv);
                            continue;
                        }

                        String[] pole = zprava.split("\\|");

                        if (convertNumber(pole[0]) != (packetNumberRecv)) {
                            recvBuffer = getRecvBuff();
                            recv = new DatagramPacket(recvBuffer, recvBuffer.length);
                            socket.receive(recv);
                            continue;
                        }
                        sendAck();
                        checked = System.currentTimeMillis();
                        main.prijatoBytu += zprava.length();
                        main.prijatoZprav++;
                        packetNumberRecv++;
                        processMessage(zprava);

                        recvBuffer = getRecvBuff();
                        recv = new DatagramPacket(recvBuffer, recvBuffer.length);
                        socket.receive(recv);
                    }

                    beh = false;

                    String info = "";
                    String hlavicka = "";
                    if (serverEnd == true) {
                        info = "Server dostal prikaz k vypnuti, nashledanou.";
                        hlavicka = "Server konci";

                    } else {
                        info = "Server zavrel spojeni, pravdepodobne byl necekane vypnut";
                        hlavicka = "Server zavrel spojeni";
                    }
                    JOptionPane.showMessageDialog(null, info, hlavicka, JOptionPane.ERROR_MESSAGE);
                    System.out.println("Server byl vypnut.");
                    closeGameWindow();
                    ovladani.dispose();
                    main.spustKlienta();

                } catch (SocketTimeoutException e) {
                }
            } catch (IOException e) {
                System.out.println("IO Exception, socket zavrenej");
                e.printStackTrace();
            }
        }
    }

    public void run() {
        System.out.println("Vlakno bezi");
        receive();
    }

    /**
     * Konstruktor tridy komunikace, ulozi parametry do lokalnich promennych a
     * naplni pole tlacitek objetky
     *
     * @param main
     * @param socket
     * @param input
     * @param output
     */
    public UdpClient(Main main, DatagramSocket socket, InetAddress address, int portCislo) {
        this.main = main;
        this.socket = socket;
        this.portCislo = portCislo;
        this.address = address;

        for (int i = 0; i < VELIKOST_PLOCHY; i++) {
            for (int j = 0; j < VELIKOST_PLOCHY; j++) {
                tlacitkaPole[i][j] = new GameButton(j, i, sirkaPolicka);
            }
        }
    }

    public void setGUI(Control ovladani) {
        this.ovladani = ovladani;
    }

    public UdpClient() {

    }

    public GameButton[][] getPolicka() {
        return tlacitkaPole;
    }

    public void setPolicka(GameButton[][] policka) {
        this.tlacitkaPole = policka;
    }

}
