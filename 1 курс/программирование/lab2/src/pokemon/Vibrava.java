package pokemon;

import move.special.BugBuzz;
import ru.ifmo.se.pokemon.Type;

public class Vibrava extends Trapinch {
    public Vibrava(String name, int level) {
        super(name, level);
        setType(Type.GROUND, Type.DRAGON);
        setStats(50, 70, 50, 50,50, 70);
        addMove(new BugBuzz());
    }
}
