package move.physical;

import ru.ifmo.se.pokemon.PhysicalMove;
import ru.ifmo.se.pokemon.Type;

public final class AquaJet extends PhysicalMove {
    public AquaJet() {
        super(Type.WATER, 40, 100, 1, 1);
    }

    @Override
    protected String describe() {
        return "использует Aqua Jet";
    }
}
