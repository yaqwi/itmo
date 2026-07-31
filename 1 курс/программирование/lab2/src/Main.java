import pokemon.*;
import ru.ifmo.se.pokemon.*;

public class Main {
    public static void main(String[] args) {
        Battle battle = new Battle();

        battle.addAlly(new Mesprit("Mesprit", 30));
        battle.addAlly(new Psyduck("Psyduck", 30));
        battle.addAlly(new Golduck("Golduck", 30));

        battle.addFoe(new Trapinch("Trapinch", 30));
        battle.addFoe(new Vibrava("Vibrava", 30));
        battle.addFoe(new Flygon("Flygon", 30));

        battle.go();
    }
}